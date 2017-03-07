//
// Created by Николай on 14/09/16.
//

#include "googletest/include/gtest/gtest.h"
#include "../lib/RedisCommands.h"
#include <vector>
#include "../lib/RedisReplyHolder.h"
#include <future>

using json = nlohmann::json;
using namespace RedisCommands;
using std::string;
using std::vector;
using std::future;
using std::async;


redisContext *redisContext1(redisConnect("127.0.0.1", 6379));


json emptyPair = {{"login",    ""},
                  {"password", ""}};

json validPair = {{"login",    "abcdefg"},
                  {"password", "abcdefg"}};

json existingPair = {{"login",    "user0"},
                     {"password", "password0"}};

json validToken = {{"token", "11413460448766762133"}};

json invalidToken = {{"token", "1451245"}};

int checkCanCreate() {
    auto *redisContext1(redisConnect("127.0.0.1", 6379));
    bool res = createAccount(redisContext1, validPair);
    redisFree(redisContext1);
    return res;
}


TEST(Commands, CreateAccountTest) {


    ASSERT_FALSE(createAccount(redisContext1, emptyPair)) << "cant create empty account";


    ASSERT_TRUE(createAccount(redisContext1, validPair)) << "valid account can be created";
    ASSERT_FALSE(createAccount(redisContext1, validPair)) << "can't create same account twice";

    auto id = RedisReplyHolder(redisContext1, "get logintoid::%s",
                               validPair["login"].dump().c_str()).getReply()->integer;
    redisCommand(redisContext1, "del logintoid::%s", "abcdefg");
    redisCommand(redisContext1, "del users::%s", std::to_string(id).c_str());
}

TEST(Commands, AuthenticateTest) {


    json wrongPassword = {{"login",    "user0"},
                          {"password", "password1"}};
    ASSERT_EQ(string(authenticate(redisContext1, emptyPair)), string("")) << "can't authenticate empty account";
    auto firstAuthRes = string(authenticate(redisContext1, existingPair));
    ASSERT_NE(firstAuthRes, string("")) << "must return non-empty token";
    ASSERT_EQ(firstAuthRes, string(authenticate(redisContext1, existingPair))) << "must return same token";

    ASSERT_EQ(string(authenticate(redisContext1, wrongPassword)), string(""))
                                << "can't authenticate account with wrong password";

}

TEST(MultiThreaded, MultiCreateTest) {

    u_long NUM_TASKS = 32;
    vector<future<int>> results(NUM_TASKS);

    for (int i = 0; i < NUM_TASKS; ++i) {
        results[i] = async(checkCanCreate);
    }
    auto sumSuccessful = 0;

    for (auto &res: results) {
        sumSuccessful += res.get();
    }

    auto id = RedisReplyHolder(redisContext1, "get logintoid::%s",
                               validPair["login"].dump().c_str()).getReply()->integer;
    redisCommand(redisContext1, "del logintoid::%s", "abcdefg");
    redisCommand(redisContext1, "del users::%s", std::to_string(id).c_str());

    ASSERT_EQ(sumSuccessful, 1) << "Only one account can be created";
}


TEST(Groups, GroupCreation) {
    auto res = 0;
    for (int i = 0; i < 5; ++i) {
        auto oldres = res;
        try {
            res = std::stoi(createNewGroup(redisContext1, validToken));

        } catch (...) {
            FAIL() << "Couldn't create group";
        }
        if (oldres > 0) {
            ASSERT_EQ(res, oldres + 1) << "Group Id must be incremented each time";
        }
    }

    ASSERT_EQ(createNewGroup(redisContext1, emptyPair).size(), 0) << "Ill-formed json";
    ASSERT_EQ(createNewGroup(redisContext1, invalidToken).size(), 0) << "Invalid token";
}


TEST(Groups, AuthenticateAndCreateGroup) {
    auto token = authenticate(redisContext1, existingPair);
    RedisReplyHolder num(redisContext1, "GET groupsCounter");
    auto res = createNewGroup(redisContext1, {{"token", token}});
    ASSERT_EQ(stoll(res), atoll(num.getReply()->str) + 1);
}

TEST(Groups, RegisterAuthenticateAndCreateGroup) {

    ASSERT_TRUE(createAccount(redisContext1, validPair)) << "valid account can be created";
    auto token = authenticate(redisContext1, validPair);
    ASSERT_NE(token.length(), 0) << "valid user can be authenticated";
    ASSERT_NE(createNewGroup(redisContext1, {{"token", token}}).length(), 0);
    auto id = RedisReplyHolder(redisContext1, "get logintoid::%s",
                               validPair["login"].dump().c_str()).getReply()->integer;
    redisCommand(redisContext1, "del logintoid::%s", "abcdefg");
    redisCommand(redisContext1, "hdel sessiontokens %s", token.c_str());
    redisCommand(redisContext1, "del users::%s", std::to_string(id).c_str());
}


TEST(Groups, GetGroupsForUser) {
    auto correct = RedisReplyHolder(redisContext1, "scard users::1::groups").getReply()->integer;
    auto actual = getAllAvailableGroups(redisContext1, validToken);
    ASSERT_EQ(correct, actual.size()) << "Cardinality must be equal to size(vector of groups)";
}

TEST(Groups, AddUserToGroup) {
    json testUser = {{"login", "login0"},
                     {"password", "password0"}};
    createAccount(redisContext1, testUser);
    auto token = authenticate(redisContext1, testUser);
    ASSERT_NE(token.length(), 0) << "Secret must be provided for correct login/password pair";
    testUser.push_back({"token", token});
    auto group_id = createNewGroup(redisContext1, testUser);
    ASSERT_NE(group_id.length(), 0) << "Group can be created by valid user";
    testUser.push_back({"group", group_id});
    testUser.push_back({"user", "1"});
    ASSERT_EQ(addUserToGroup(redisContext1, testUser), true) << "Can add valid user to group";
    deleteAccount(redisContext1, testUser);

}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    RUN_ALL_TESTS();


    redisFree(redisContext1);
    return 0;
}

