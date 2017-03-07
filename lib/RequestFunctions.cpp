//
// Created by Николай on 10/10/16.
//

#include <hiredis/hiredis.h>
#include <memory>
#include <evhttp.h>
#include <third_party/json.hpp>
#include <third_party/easylogging++.h>
#include <evhtp.h>
#include "RequestFunctions.h"
#include "RedisCommands.h"
#include "util.h"



/*
 * static void MessagesRequestHandle(evhttp_request *request, void *data) {
        LOG(INFO) << "handling request:\n" << evhttp_request_get_connection(request);
        auto hld = reinterpret_cast<HttpEvHolder *>(data);
        json body;
        try {
            body = json::parse(MessageParser::getRequestBody(request));
        } catch (...) {
            hld->sendErrorReply(request);
            return;
        }
        RedisCommands::getAllAvailableGroups(hld->redContext(), )
    }
*/
namespace reqproc {

    using nlohmann::json;

    using std::unordered_map;
    using std::string;
    using namespace util;

    std::unordered_map<std::string, evhtp_request_t *> pending_requests;

    std::mutex mtx;


    static evhtp_res fini_cb(evhtp_request_t *request, void *arg) {
        auto id = reinterpret_cast<string*>(arg);
        std::lock_guard<mutex> guard{mtx};
        pending_requests.erase(*id);
        return EVHTP_RES_OK;
    }


    void makeReplies(const vector<evhtp_request_t *> &requests) {
        for (auto request: requests) {
            evhtp_request_resume(request);
        }
    }

    void flushBuffer(evbuffer *buf) {
        evbuffer_drain(buf, evbuffer_get_length(buf));
    }

    void freeRequest(evhtp_request_t *req) {
        evhtp_unset_all_hooks(&req->hooks);
        evbuffer_add_printf(req->buffer_out, "\r\n");
        evhtp_send_reply(req, EVHTP_RES_OK);
        evhtp_request_resume(req);
    }



    void broadcast(const vector<string> &members) {
        vector<evhtp_request_t *> to_proceed;
        {
            std::lock_guard<std::mutex> lock{mtx};
            for (const auto &memb: members) {
                if (pending_requests.find(memb) != pending_requests.end()) {
                    auto req = pending_requests[memb];
                    to_proceed.emplace_back(req);
                    pending_requests.erase(memb);
                }
            }
        }
        makeReplies(to_proceed);
    }

    void newGroupHandle(redisContext *redContext, const json &body, evhtp_request_t *request) {
        auto res = RedisCommands::createNewGroup(redContext, body);
        evbuffer_add_printf(request->buffer_out, "%s", json{{"result", res}}.dump().c_str());
        evhtp_send_reply(request, EVHTP_RES_OK);
    }


    void newMessageHandle(redisContext *redContext, const json &body, evhtp_request_t *request) {
        const auto &groups = RedisCommands::getAllAvailableGroups(redContext, body);
        auto answ = json::object();
        if (std::find(groups.begin(), groups.end(), getKeyValue(body, "group")) == groups.end()) {
            answ[getKeyValue(body, "localId")] = "";
            evbuffer_add_printf(request->buffer_out, "%s", answ);
            evhtp_send_reply(request, EVHTP_RES_OK);
            return;
        }
        auto res = RedisCommands::addMessageToGroup(redContext, body);
        LOG(INFO) << getKeyValue(body, "localId");
        answ[getKeyValue(body, "localId")] = res.second;
        evbuffer_add_printf(request->buffer_out, "%s", answ.dump().c_str());
        evhtp_send_reply(request, EVHTP_RES_OK);
        if (res.first) {
            const auto &waiting = RedisCommands::getWaitingMembers(redContext, getKeyValue(body, "group"));
            std::lock_guard<mutex> lock{mtx};
            for (const auto &member: waiting) {
                if (member != getKeyValue(body, "token"))
                    if (pending_requests.count(member)) {
                        auto req = pending_requests[member];
                        pending_requests.erase(member);
                        evbuffer_add_printf(req->buffer_out, "{\"updates\": [\"%s\"]}", res.second.c_str());
                        freeRequest(req);
                    }
            }
        }
    }


    void pollingHandle(redisContext *redContext, const json &body, evhtp_request_t *request) {
        const auto &full_id = getKeyValue(body, "token");// + ":" + getKeyValue(body, "app_id");

        //const auto &upds = RedisCommands::getPendingUpdates(redContext, full_id);
        vector<string> reply{};
        if (body["groups"][0][0].dump() != "-1")
            for (auto &groupMessages: body["groups"]) {
                const auto &msgs = RedisCommands::getUnreadMessages(redContext,
                                                                    groupMessages[1].dump(),
                                                                    groupMessages[0].dump());
                reply.insert(std::end(reply), std::begin(msgs), std::end(msgs));
            }
        if (reply.size()) {
            const auto &res = json{{"updates", reply}}.dump();
            evbuffer_add_printf(request->buffer_out, "%s", res.c_str());
            evhtp_send_reply(request, EVHTP_RES_OK);
        } else {
            {
                RedisCommands::insertWaitingMember(redContext, full_id, body["groups"][0][0].dump());
                std::lock_guard<std::mutex> lock{mtx};
                if (pending_requests.count(full_id)) {
                    freeRequest(pending_requests[full_id]);
                    pending_requests.erase(full_id);
                }
                pending_requests[full_id] = request;
                evhtp_set_hook(&request->hooks, evhtp_hook_on_request_fini, (evhtp_hook)fini_cb,
                               (void*)&pending_requests.find(full_id)->first);
                evhtp_request_pause(request);
                evhtp_connection_resume(request->conn);
            }
        }
    }


    void groupsRequestHandle(redisContext *redContext, const json &body, evhtp_request_t *request) {
        json res = {{"groups", RedisCommands::getAllAvailableGroups(redContext, body)}};
        evbuffer_add_printf(request->buffer_out, "%s", res.dump().c_str());
        evhtp_send_reply(request, EVHTP_RES_OK);
    }


    void loginRequestHandle(redisContext *redContext, const json &body, evhtp_request_t *request) {
        std::string token = RedisCommands::authenticate(redContext, body);
        if (token == "") {
            evbuffer_add_printf(request->buffer_out, "Error. Incorrect login/password pair");
        } else {
            json response = {{"token", token}};
            evbuffer_add_printf(request->buffer_out, "%s", response.dump().c_str());
        }
        evhtp_send_reply(request, EVHTP_RES_OK);
        std::lock_guard<mutex> guard(mtx);
        pending_requests.erase(token.substr(0, token.find(":")));
    }


    void signUpRequestHandle(redisContext *redContext, const json &body, evhtp_request_t *request) {
        bool res = RedisCommands::createAccount(redContext, body);
        json answer = {{"result", res}};
        evbuffer_add_printf(request->buffer_out, "%s", answer.dump().c_str());
        evhtp_send_reply(request, EVHTP_RES_OK);
    }

    void groupMembersHandle(redisContext *redContext, const json &body, evhtp_request_t *request) {
        const auto &res = RedisCommands::getUsersByGroup(redContext, body, false, false);
        const auto &admins = res.size()? RedisCommands::getAdmins(redContext, body) : "";
        const auto &answer = json{{"result", res},
                                  {"admins", admins},
                                  {"group",  body["group"]}};
        evbuffer_add_printf(request->buffer_out, "%s", answer.dump().c_str());
        evhtp_send_reply(request, EVHTP_RES_OK);
    }

    //also handles remove
    void addMemberHandle(redisContext *redContext, const json &body, evhtp_request_t *request) {
        const auto &id = RedisCommands::loginToId(redContext, body);
        if (id == "") {
            evbuffer_add_printf(request->buffer_out, "%s", "wrong login");
            evhtp_send_reply(request, EVHTP_RES_OK);
            return;
        }
        auto name = body["user"];
//        body["user"] = id;
        bool res;
        if (getKeyValue(body, "method") == "remove") {
            res = RedisCommands::removeUserFromGroup(redContext, json{{"user",  id},
                                                                      {"group", body["group"]},
                                                                      {"token", body["token"]}});
        } else {
            res = RedisCommands::addUserToGroup(redContext, json{{"user",  id},
                                                                 {"group", body["group"]},
                                                                 {"token", body["token"]}});
        }

        const auto &answer = json{{"result", res ? "OK" : "FAIL"},
                                  {"group",  body["group"]},
                                  {"member", name}};
        LOG(INFO) << answer.dump(4);
        LOG(INFO) << evbuffer_get_length(request->buffer_out);
        flushBuffer(request->buffer_out);
        evbuffer_add_printf(request->buffer_out, "%s", answer.dump().c_str());
        evhtp_send_reply(request, EVHTP_RES_OK);

        if (res) {
            const auto &members = RedisCommands::getUsersByGroup(redContext, body, true, true);
            std::lock_guard<mutex> guard{mtx};
            for (const auto &member: members) {
                const auto &token = RedisCommands::getSessionToken(redContext, member, false);
                LOG(INFO) << token;
                if (token != "" && token != getKeyValue(body, "token")
                    && pending_requests.count(token)) {
                        auto req = pending_requests[token];
                        pending_requests.erase(token);
                        evbuffer_add_printf(req->buffer_out, "{\"grUpdates\": [\"%s\"]}",
                                            getKeyValue(body, "group").c_str());
                        freeRequest(req);
                    }
            }
        }
    }

    void messageContentHandle(redisContext *redContext, const json &body, evhtp_request_t *request) {
        const auto &id = RedisCommands::getIdBySecret(redContext, body);
        if (id == "") {
            evbuffer_add_printf(request->buffer_out, "uncrecognized token");
            evhtp_send_reply(request, EVHTP_RES_OK);
        }
        const auto &arguments = json{{"id",       id},
                                     {"messages", body["messages"]}};
        const auto &res = RedisCommands::getMessageContent(redContext, arguments);
        flushBuffer(request->buffer_out);
        evbuffer_add_printf(request->buffer_out, "%s", res.dump().c_str());
        evhtp_send_reply(request, EVHTP_RES_OK);
    }

}