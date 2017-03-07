#include "serverconnector.h"
#include <QSettings>
#include <assert.h>

namespace connect {

ServerConnector::ServerConnector()
{
    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
}

ServerConnector* ServerConnector::Instance() {
    static ServerConnector instance;
    return &instance;
}

void ServerConnector::loginRequest(const QString &login, const QString &password) {
    QVariantHash args;
    settings_.setValue("login", login);
    args["login"] = login;
    args["password"] = password;
    auto app_id = getAppId();
    if (app_id != "") {
        args["app_id"] = app_id.toInt();
    }
    abstractRequest("login", args, ReplyTypes::login);
}

void ServerConnector::signUpRequest(const QString &login, const QString &password) {
    QVariantHash args;
    args["login"] = login;
    args["password"] = password;
    abstractRequest("signup", args, ReplyTypes::signup);
}

void ServerConnector::getGroupsRequest(const QString &token) {
    QVariantHash args;
    args["token"] = token;
    abstractRequest("groups", args, ReplyTypes::getgroups);
}


void ServerConnector::getUpdatesRequest(int groupId, int msgsCount,
                                        const QString &token, const QString &appId) {
    QJsonArray groups;
    groups.append(QJsonArray{groupId, msgsCount + 1});
    QVariantHash args = {
        {"groups", groups},
        {"token", token},
        {"app_id", appId}
    };
    abstractRequest("poll", args, ReplyTypes::getupdates);
}

void ServerConnector::addMemberRequest(const QString &memberName, const QString &groupId,
                                       const QString &method,
                                       const QString &token, const QString &appId) {
    QVariantHash args = {
        {"token", token},
        {"app_id", appId},
        {"group", groupId},
        {"user", memberName},
        {"method", method}
    };
    if (method == "add") abstractRequest("add_member_to_gr", args, ReplyTypes::addmember);
    else abstractRequest("add_member_to_gr", args, ReplyTypes::removemember);
}

void ServerConnector::sendMessageRequest(const QString &group, const QString &content,
                                         int localId,
                                         const QString &token, const QString &appId) {
    QVariantHash args = {
        {"token", token},
        {"app_id", appId},
        {"group", group},
        {"content", content},
        {"localId", QString::number(localId)}
    };
    abstractRequest("new_message", args, ReplyTypes::sendmessage);
}

void ServerConnector::getGroupMembersRequest(const QString &groupId,
                                             const QString &token, const QString &appId){
    QVariantHash args = {
        {"token", token},
        {"app_id", appId},
        {"group", groupId}
    };
    abstractRequest("groupmembers", args, ReplyTypes::groupusers);
}

void ServerConnector::getMessagesContentsRequest(const QJsonArray &ids,
                                                 const QString &token, const QString &appId) {
    QVariantHash args = {
        {"token", token},
        {"app_id", appId},
        {"messages", ids}
    };
    abstractRequest("msg_content", args, ReplyTypes::messagecontents);
}

void ServerConnector::addGroupRequest(const QString &token, const QString &appId) {
    QVariantHash args = {
        {"token", token},
        {"app_id", appId},
    };
    abstractRequest("new_group", args, ReplyTypes::addgroup);
}

void ServerConnector::contentsReply(const QJsonObject &reply) {
    qDebug()<< "messages: " << reply;
    if (!reply.isEmpty())
        emit Instance()->messages(reply);
}

void ServerConnector::groupsReply(const QJsonObject &reply) {
    auto ans = reply["groups"].toArray();
    emit Instance()->newGroups(ans);
}

void ServerConnector::groupUsersReply(const QJsonObject &reply) {
    qDebug() << "reply result: " << reply["result"];
    if (reply["result"].toArray().isEmpty()) return;
    emit Instance()->receivedGroupMembers(reply["result"].toArray(), reply["group"].toString());
    emit Instance()->receivedGroupAdmins(reply["admins"].toString(), reply["group"].toString());
}

void ServerConnector::loginReply(const QJsonObject &reply) {
    const auto &ans = (!reply["token"].isNull())?reply["token"].toString():"";
    if (ans != "") {
        Instance()->setToken(ans);
    }
    emit Instance()->loginSuccess(ans);
}

void ServerConnector::signupReply(const QJsonObject &reply) {
    emit Instance()->signupSuccess(reply["result"].isBool() && reply["result"].toBool());
}



void ServerConnector::updatesReply(const QJsonObject &reply) {
    if (reply.isEmpty()) return;
    if (reply["updates"].isArray()) {
        qDebug() << reply["updates"].toArray();
        Instance()->getMessagesContentsRequest(reply["updates"].toArray());
    }
    if (reply["grUpdates"].isArray()) {
        emit Instance()->newGroups(reply["grUpdates"].toArray());
        qDebug() << reply["grUpdates"].toArray();
    }
}

void ServerConnector::sendMessageReply(const QJsonObject &reply) {
    qDebug() << "send message, reply result: " << reply;
    if (!reply.isEmpty())
        emit Instance()->messageAccepted(reply);
}

void ServerConnector::addMemberReply(const QJsonObject &reply) {
    if (reply["result"] == "OK") {
        emit Instance()->memberAdded(reply["member"].toString(), reply["group"].toString());
    }
}

void ServerConnector::removeMemberReply(const QJsonObject &reply) {
    if (reply["result"] == "OK") {
        emit Instance()->memberRemoved(reply["member"].toString(), reply["group"].toString());
    }
}

void ServerConnector::addGroupReply(const QJsonObject &reply)
{
    if (reply["result"] != "") {
        emit Instance()->newGroups(QJsonArray({reply["result"]}));
    }
}

void setHeaders(QNetworkRequest* request, int bodySize) {
    request->setRawHeader("User-Agent", "client v1");
    request->setRawHeader("X-Custom-User-Agent", "client v1");
    request->setRawHeader("Content-Type", "application/json");
    request->setRawHeader("Connection", "Keep-Alive");
    request->setRawHeader("Content-Length", QByteArray::number(bodySize));
}

void ServerConnector::abstractRequest(const QString &urlPath, const QVariantHash &args,
                                                ReplyTypes type,
                                                const QString &host, int port) {
    QUrl url(host + urlPath);
    url.setPort(port);
    QJsonObject toSend = QJsonObject::fromVariantHash(args);
    qDebug() << toSend;
    const auto &body = QJsonDocument(toSend).toJson(QJsonDocument::Compact);
    QNetworkRequest request(url);
    setHeaders(&request, body.size());
    auto repl = manager->post(request, body);
    replyType[repl] = type;
}
} //connect
