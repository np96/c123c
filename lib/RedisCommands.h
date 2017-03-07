//
// Created by Николай on 31/08/16.
//

#ifndef CHAT_REDISCOMMANDS_H
#define CHAT_REDISCOMMANDS_H

#include <hiredis/hiredis.h>
#include "third_party/json.hpp"
#include <vector>
#include <mutex>

using json = nlohmann::json;
using mutex = std::mutex;
using std::string;
using std::vector;
namespace RedisCommands {






    string loginToId(redisContext *, const json &);

    std::pair<bool, string> checkLoginPassword(redisContext *, const json &);

    string getSessionToken(redisContext *, const string &, bool need_app_key = true);

    string generateSessionToken(redisContext *, const string &);

    string authenticate(redisContext *, const json &);

    vector<string> getAllAvailableGroups(redisContext *, const json &);

    string createNewGroup(redisContext *, const json &);

    string generateAppKey(redisContext *, const string &);

    bool addUserToGroup(redisContext *, const json &);

    bool createAccount(redisContext *, const json &);

    bool deleteAccount(redisContext *, const json &);

    string getIdBySecret(redisContext *, const json &);

    std::pair<bool, string> addMessageToGroup(redisContext *, const json &);

    vector<string> getMessagesByGroup(redisContext *, const json &);

    json getMessageContent(redisContext *, const json &);

    vector<string> getUsersByGroup(redisContext *, const json &, bool, bool);

    vector<string> insertUpdate(redisContext *, const string &, const string &, const string &);

    json getPendingUpdates(redisContext *, const string &);

    bool updateSingleApp(redisContext *redContext, const string &app_id, const string &group_id);

    vector<string> getUnreadMessages(redisContext *redContext, const string &last_message, const string &group_id);

    bool insertWaitingMember(redisContext *redContext, const string &full_id, const string &group_id);

    vector<string> getWaitingMembers(redisContext *redContext, const string &group_id);

    bool removeUserFromGroup(redisContext *redContext, const json &body);

    string getAdmins(redisContext *redContext, const json &body);

};


#endif //CHAT_REDISCOMMANDS_H
