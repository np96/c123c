//
// Created by Николай on 31/08/16.
//

#include <chrono>

#include "RedisCommands.h"
#include "third_party/easylogging++.h"
#include "RedisReplyHolder.h"
#include "LoginPasswordPair.h"
#include "util.h"

using namespace std::placeholders;


using std::string;
using std::vector;

using std::lock_guard;
using std::mutex;
using std::pair;
using std::chrono::system_clock;


using namespace util;
namespace { mutex signUpMutex; }



static bool valid(redisContext *redContext, const json &body, string& user_id, string& group_id, string& added) {
    user_id = RedisCommands::getIdBySecret(redContext, body);

    group_id = getKeyValue(body, "group");
    added = getKeyValue(body, "user");

    if (group_id.length() == 0 || user_id.length() == 0 || added.length() == 0) {
        return false;
    }

    auto is_admin = RedisReplyHolder(redContext, "SISMEMBER groups::%s::groupadmins %s", group_id.c_str(),
                                     user_id.c_str()).getReply()->integer == 1;
    return is_admin;

}


pair<bool, string> RedisCommands::checkLoginPassword(redisContext *redContext, const json &body) {
    LoginPasswordPair login_password(body);
    if (!login_password) {
        return make_pair(false, string(""));
    }

    RedisReplyHolder reply(redContext, "GET logintoid::%s", login_password.getLogin().c_str());

    if (reply.type() == REDIS_REPLY_NIL) return make_pair(false, string(""));

    auto id = std::string(reply.getReply()->str);
    RedisReplyHolder password_reply(redContext, "HGET users::%s password", id.c_str());

    return make_pair(strcmp(login_password.getPassword().c_str(), password_reply.getReply()->str) == 0, id);
}


string RedisCommands::generateAppKey(redisContext *redContext, const string &user_id) {
    RedisReplyHolder reply(redContext, {"INCR users::", user_id, "::appcounter"});
    return std::to_string(reply.getReply()->integer);
}

string RedisCommands::getSessionToken(redisContext *redContext, const string &user_id, bool need_app_key) {
    RedisReplyHolder reply(redContext, "HGET users::%s token", user_id.c_str());
    string key = "";
    if (need_app_key) {
        key += ":" + generateAppKey(redContext, user_id);
    }
    auto isNil = reply.type() == REDIS_REPLY_NIL;
    return isNil ? generateSessionToken(redContext, user_id) + key : string(reply.getReply()->str) + key;
}


string RedisCommands::generateSessionToken(redisContext *redContext, const string &user_id) {
    auto hash = std::hash<std::string>()(user_id) + std::hash<time_t>()(time(0));
    redisCommand(redContext, "HSET sessiontokens %s %s", std::to_string(hash).c_str(), user_id.c_str());
    redisCommand(redContext, "HSET users::%s token %s", user_id.c_str(), std::to_string(hash).c_str());
    return std::to_string(hash);
}

string RedisCommands::authenticate(redisContext *redContext, const json &body) {
    auto res = checkLoginPassword(redContext, body);
    if (!res.first) {
        return "";
    }
    bool existing_app = body.count("app_id") != 0                                       //request contains app id
                        && body["app_id"].is_number() && body["app_id"].get<int>() > 0;  //it is positive integer
    RedisReplyHolder get_counter(redContext, {"GET users::", res.second, "::appcounter"});
    existing_app &= get_counter.type() == REDIS_REPLY_INTEGER
                    && get_counter.integer() <= body["app_id"].get<int>();   //and such app id exists
    return getSessionToken(redContext, res.second, !existing_app);
}


bool RedisCommands::createAccount(redisContext *redContext, const json &body) {

    LoginPasswordPair login_password(body);
    if (!login_password) {
        return false;
    }
    auto command = std::bind(redisCommand, redContext, _1);
    auto login = login_password.getLogin().c_str();
    std::lock_guard<mutex> guard{signUpMutex};

    //First we check if such login already exists
    //vector<string> lol;

    RedisReplyHolder replyHolder(redContext, "EXISTS logintoid::%s", login);
    //1 if exists, 0 if not
    if (replyHolder.getReply()->integer == 1) {
        return false;
    }

    auto id = RedisReplyHolder(command("INCR accountcounter")).getReply()->integer;

    if (RedisReplyHolder(redContext, "SET logintoid::%s %s", login, std::to_string(id).c_str()).getReply()->type !=
        REDIS_REPLY_STATUS) {
        //something failed
        return false;
    }
    //set id -> login, password
    RedisReplyHolder(redContext, "HMSET users::%s login %s password %s", std::to_string(id).c_str(), login,
                     login_password.getPassword().c_str());
    return true;
}

vector<string> RedisCommands::getAllAvailableGroups(redisContext *redContext, const json &body) {
    auto id = getIdBySecret(redContext, body);
    if (id.length() == 0) {
        return {};
    }
    RedisReplyHolder result(redContext, "SMEMBERS users::%s::groups", id.c_str());

    if (result.type() != REDIS_REPLY_ARRAY) {
        return {};
    }
    return result.getReplyArray();
}

string RedisCommands::getIdBySecret(redisContext *redContext, const json &body) {
    auto it = body.find("token");
    if (it == body.end()) return "";

    auto len = it.value().dump().length() - 2;
    auto ii = it.value().dump().substr(1, len);

    RedisReplyHolder res(redContext, "HGET sessiontokens %s", ii.c_str());
    if (res.type() != REDIS_REPLY_STRING) {
        return "";
    }
    return res.getReply()->str;
}

string RedisCommands::createNewGroup(redisContext *redContext, const json &body) {
    //lookup id by secret
    auto user_id = getIdBySecret(redContext, body);
    if (user_id.length() == 0) {
        return "";
    }

    //make new group, increment counter
    auto group_id = std::to_string(RedisReplyHolder(redContext, "INCR groupsCounter").getReply()->integer);

    //add group creator to admins and members
    RedisReplyHolder(redContext, "SADD groups::%s::groupadmins %s", group_id.c_str(), user_id.c_str());
    auto time = std::to_string(system_clock::now().time_since_epoch().count()).c_str();
    RedisReplyHolder(redContext, "ZADD groups::%s::groupmembers %s %s", group_id.c_str(), 0, user_id.c_str());
    RedisReplyHolder(redContext, "SADD users::%s::groups %s", user_id.c_str(), group_id.c_str());
    return group_id;
}


bool RedisCommands::addUserToGroup(redisContext *redContext, const json &body) {
    std::string user_id = "", group_id = "", added = "";
    if (!valid(redContext, body, user_id, group_id, added)) return false;

    return RedisReplyHolder(redContext, "SADD users::%s::groups %s", added.c_str(),
                            group_id.c_str()).getReply()->integer &&
           RedisReplyHolder(redContext, "ZADD groups::%s::groupmembers %s %s", group_id.c_str(), currentTime().c_str(),
                            added.c_str()).getReply()->integer;
}

bool RedisCommands::removeUserFromGroup(redisContext *redContext, const json &body) {
    std::string user_id = "", group_id = "", removed = "";
    if (!valid(redContext, body, user_id, group_id, removed)) return false;
    return RedisReplyHolder(redContext, "SREM users::%s::groups %s", removed.c_str(),
                            group_id.c_str()).getReply()->integer &&
           RedisReplyHolder(redContext, "ZREM groups::%s::groupmembers %s %s", group_id.c_str(), currentTime().c_str(),
                            removed.c_str()).getReply()->integer;
}


bool RedisCommands::deleteAccount(redisContext *redContext, const json &body) {
    auto id = getIdBySecret(redContext, body);
    LoginPasswordPair login_password(body);
    if (id.length() == 0 || !login_password) return false;
    if (!checkLoginPassword(redContext, body).first) return false;
    bool res = 1;

    res &= RedisReplyHolder(redContext, "DEL users::%s", id.c_str()).getReply()->integer;
    res &= RedisReplyHolder(redContext, "HDEL sessiontokens %s", body["token"].dump().c_str()).getReply()->integer;
    res &= RedisReplyHolder(redContext, "DEL logintoid::%s", login_password.getLogin().c_str()).getReply()->integer;
    return res;
}

pair<bool, string> RedisCommands::addMessageToGroup(redisContext *redContext, const json &body) {
    auto id = getIdBySecret(redContext, body);
    auto group_id = getKeyValue(body, "group");
    auto msg_body = getKeyValue(body, "content");


    if (id.length() == 0 || group_id.length() == 0 || msg_body.length() == 0) return std::make_pair(false, "");

    auto msg_id = std::to_string(RedisReplyHolder(redContext, "INCR messagesCounter").getReply()->integer);

    vector<string> grReq = {"LPUSH groups::", group_id, "::messages ", msg_id};
    RedisReplyHolder(redContext, grReq);

    vector<string> req = {msg_id, msg_body, currentTime(), id, group_id};
    auto msg = RedisReplyHolder::messageCommand(redContext, req);
    return make_pair(msg.get()->getReply()->type == REDIS_REPLY_STATUS &&
                     strcmp(msg.get()->getReply()->str, "OK") == 0, msg_id);
}

vector<string> RedisCommands::getMessagesByGroup(redisContext *redContext, const json &body) {
    auto id = getIdBySecret(redContext, body);
    auto group_id = getKeyValue(body, "group");
    auto start = getKeyValue(body, "page");
    if (id.length() == 0 || group_id.length() == 0) return {};

    auto messages_start = (isNumber(start)) ? std::max(atoll(start.c_str()) * 50, 0LL) : 0LL;

    vector<string> req = {"LRANGE groups::", group_id, "::messages ", std::to_string(messages_start), " ",
                          std::to_string(messages_start + 50)};
    RedisReplyHolder reply(redContext, req);

    vector<string> res = {};
    for (int i = 0; i < reply.getReply()->elements; ++i) {
        res.emplace_back(reply.getReply()->element[i]->str);
    }

    return res;
}

vector<string> RedisCommands::getUsersByGroup(redisContext *redContext, const json &body, bool is_system = false, bool ids = false) {
    auto id = getIdBySecret(redContext, body);
    auto group_id = getKeyValue(body, "group");
    if ((id.length() == 0 && !is_system) || group_id.length() == 0) return {};

    vector<string> req = {"ZRANGE groups::", group_id, "::groupmembers 0 -1"};

    RedisReplyHolder reply(redContext, req);
    if (ids) {
        return reply.getReplyArray();
    }
    vector<string> nameReq = {"HGET users::", "temp", " login"};
    vector<string> res{};
    for (int i = 0; i < reply.getReply()->elements; ++i) {
        nameReq[1] = reply.getReply()->element[i]->str;
        res.emplace_back(
                RedisReplyHolder(redContext, nameReq)
                        .getReply()->str);
    }
    return res;
}

vector<string> RedisCommands::insertUpdate(redisContext *redContext, const string &user_id, const string &msg_id,
                                           const string &sentby_id) {
    vector<string> req = {"LRANGE users::", user_id, "::apptokens", " 0 -1"};
    vector<string> ans;
    RedisReplyHolder reply(redContext, req);
    auto tokens(reply.getReplyArray());
    std::for_each(tokens.begin(), tokens.end(), [&ans, redContext, &user_id, &sentby_id, &msg_id](string token) {
        if (token != sentby_id) {
            RedisCommands::updateSingleApp(redContext, user_id + "::" + token, msg_id);
            ans.push_back(user_id + "::" + token);
        }
    });
    return ans;
}

bool RedisCommands::updateSingleApp(redisContext *redContext, const string &full_id, const string &msg_id) {
    vector<string> req1 = {"LPUSH users::", full_id, "::queue ", msg_id};
    RedisReplyHolder(redContext, req1);
    return true;
}

json RedisCommands::getPendingUpdates(redisContext *redContext, const string &full_id) {
    vector<string> req1 = {"LRANGE users::", full_id, "::queue 0 -1"};
    const auto &reply = RedisReplyHolder(redContext, req1);
    json res = json::array();
    vector<string> msgRequest = {"HGETALL messages::", ""};
    std::for_each(reply.getReplyArray().begin(), reply.getReplyArray().end(),
                  [&res, &msgRequest, redContext](const string &update) {
                      msgRequest[1] = update;
                      RedisReplyHolder msgReply(redContext, msgRequest);
                      res.push_back(msgReply.getJsonReply());
                  });
    return res;
}

string RedisCommands::loginToId(redisContext *redContext, const json &body) {
    if (getKeyValue(body, "user") == "") return "";
    vector<string> req = {"GET logintoid::", body["user"]};
    RedisReplyHolder reply{redContext, req};
    if (reply.type() == REDIS_REPLY_NIL) return "";
    return reply.getReply()->str;
}

json RedisCommands::getMessageContent(redisContext *redContext, const json &body) {
    json result{};
    for (const auto &id: body["messages"]) {
        vector<string> reqMsg = {"HGETALL messages::", id};
        RedisReplyHolder msgHolder(redContext, reqMsg);
        auto msg = msgHolder.getJsonReply();
        LOG(INFO) << msg;
        vector<string> reqContains = {"ZRANK groups::", msg["group"], "::groupmembers ", body["id"]};
        RedisReplyHolder containsMember(redContext, reqContains);
        if (containsMember.getReply()->type == REDIS_REPLY_NIL) {
            return json();
        }
        vector<string> loginReq = {"HGET users::", msg["author"], " login"};
        RedisReplyHolder loginHolder(redContext, loginReq);
        const string &login = loginHolder.getReply()->str;
        msg["author"] = login;
        result[id.dump()] = msg;
    }
    return result;
}

vector<string> RedisCommands::getUnreadMessages(redisContext *redContext, const string &last_message_id, const string &group_id) {
    //hgetall groups::42:: 0 -42 — gets messsage id's from last to the asked
    vector<string> req = {"LRANGE groups::", group_id, "::messages ",  "0 -", last_message_id};
    RedisReplyHolder hold(redContext, req);
    return hold.getReplyArray();
}

bool RedisCommands::insertWaitingMember(redisContext *redContext, const string &full_id, const string &group_id) {
    vector<string> request = {"SADD groups::", group_id, "::waiting ", full_id};
    return bool(RedisReplyHolder(redContext, request).integer());
}

vector<string> RedisCommands::getWaitingMembers(redisContext *redContext, const string &group_id){
    vector<string> request = {"SMEMBERS groups::", group_id, "::waiting"};
    return RedisReplyHolder(redContext, request).getReplyArray();
}

string RedisCommands::getAdmins(redisContext* redContext,const json &body) {
    auto group_id = getKeyValue(body, "group");
    vector<string> request = {"SMEMBERS groups::", group_id, "::groupadmins"};
    vector<string> request2 = {"HGET users::", "temp", " login"};
    for (const auto& id: RedisReplyHolder(redContext, request).getReplyArray()) {
        //todo: now only 1 admin supported!!
        request2[1] = id;
        return RedisReplyHolder(redContext, request2).getReply()->str;
    }
}
