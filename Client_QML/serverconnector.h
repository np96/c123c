#ifndef SERVERCONNECTOR_H
#define SERVERCONNECTOR_H
#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QSet>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QJsonArray>
//#include <updatesreceiver.h>
#include "serverconnector.h"
//#include "update.h"
#include <memory>


namespace connect {

enum class ReplyTypes : uint {
    signup = 0, login,
    getgroups, getupdates,
    sendmessage, groupusers,
    addmember, removemember,
    addgroup, messagecontents
};

inline uint qHash(const ReplyTypes &key, uint seed) {
    return ::qHash(static_cast<uint>(key), seed);
}

class ServerConnector: public QObject
{
    Q_OBJECT
public:

    QSettings settings_{"cppqtg", "chat"}; //todo: move somewhere else

    ServerConnector(const ServerConnector&) = delete;
    ServerConnector& operator=(const ServerConnector&) = delete;
    ServerConnector(ServerConnector&&) = delete;
    ServerConnector& operator=(ServerConnector&&) = delete;

    ~ServerConnector() = default;

    static ServerConnector* Instance();

    void setToken(const QString &res) {
        if (res == "null") {
            settings_.remove("token");
            settings_.remove("app_id");
            return;
        }
        auto pair = res.split(":");
        if (pair.size() > 0 && pair[0] != "") {
            settings_.setValue("token", pair[0]);
        }
        if (pair.size() > 1 && pair[1] != "") {
            settings_.setValue("app_id", pair[1]);
        }
    }

    QString getToken() {
        return settings_.value("token", "").toString();
    }

    QString getAppId() {
        return settings_.value("app_id", "").toString();
    }

    Q_INVOKABLE QString login() {
        return settings_.value("login").toString();
    }

    Q_INVOKABLE void loginRequest(const QString &login, const QString &password);

    Q_INVOKABLE void signUpRequest(const QString &login, const QString &password);

    Q_INVOKABLE void getGroupsRequest(const QString &token = Instance()->getToken());

    Q_INVOKABLE void getUpdatesRequest(int groupId, int msgsCount,
            const QString &token = Instance()->getToken(), const QString& appId = Instance()->getAppId());

    Q_INVOKABLE void addMemberRequest(const QString &memberId, const QString &groupId,
                                      const QString &method = "add"
            ,const QString &token = Instance()->getToken(), const QString &appId = Instance()->getAppId());

    Q_INVOKABLE void sendMessageRequest(const QString &group, const QString &content, int localId,
            const QString &token = Instance()->getToken(), const QString &appId = Instance()->getAppId());

    Q_INVOKABLE void getGroupMembersRequest(const QString &groupId,
            const QString &token = Instance()->getToken(), const QString &appId = Instance()->getAppId());

    Q_INVOKABLE void getMessagesContentsRequest(const QJsonArray &ids,
            const QString &token = Instance()->getToken(), const QString &appId = Instance()->getAppId());

    Q_INVOKABLE void addGroupRequest(
            const QString &token = Instance()->getToken(), const QString &appId = Instance()->getAppId());



signals:
    void loginSuccess(const QString &ans);
    void signupSuccess(bool ans);
    void newGroups(const QJsonArray &ans);
    //void newUpdate(std::shared_ptr<Update> upd);
    void messageAccepted(const QJsonObject &reply);
    void messages(const QJsonObject &reply);
    void newMessage(const QJsonObject &message, const QString& id);
    void receivedGroupMembers(const QJsonArray &members, const QString &group);
    void receivedGroupAdmins(const QString &admins, const QString &group);
    void memberAdded(const QString &member, const QString &group);
    void memberRemoved(const QString &member, const QString &group);

private slots:

    void replyFinished(QNetworkReply* reply) {
        auto type = replyType.find(reply);
        auto temp = reply->readAll();
        qDebug() << temp;
        const auto& doc = QJsonDocument::fromJson(temp);
        qDebug()<<doc.object()<< " received";
        replyFunctions[type.value()](doc.object());
        replyType.erase(type);
        reply->deleteLater();
    }


private:

    static void contentsReply(const QJsonObject &reply);

    static void groupsReply(const QJsonObject &reply);

    static void loginReply(const QJsonObject &reply);

    static void signupReply(const QJsonObject &reply);

    static void updatesReply(const QJsonObject &reply);

    static void sendMessageReply(const QJsonObject &reply);

    static void groupUsersReply(const QJsonObject &reply);

    static void addMemberReply(const QJsonObject &reply);

    static void addGroupReply(const QJsonObject &reply);

    static void removeMemberReply(const QJsonObject &reply);

    QHash<QNetworkReply*, ReplyTypes> replyType;

    QHash<ReplyTypes,std::function<void(const QJsonObject&)>> replyFunctions =
    {
        {ReplyTypes::getgroups, &groupsReply},
        {ReplyTypes::login, &loginReply},
        {ReplyTypes::signup, &signupReply},
        {ReplyTypes::getupdates, &updatesReply},
        {ReplyTypes::messagecontents, &contentsReply},
        {ReplyTypes::sendmessage, &sendMessageReply},
        {ReplyTypes::groupusers, &groupUsersReply},
        {ReplyTypes::addmember, &addMemberReply},
        {ReplyTypes::removemember, &removeMemberReply},
        {ReplyTypes::addgroup, &addGroupReply}
    };

    QNetworkAccessManager *manager;

    const static int defaultPort = 6755;

    ServerConnector();

    void abstractRequest(const QString &urlPath, const QVariantHash &args,
                         ReplyTypes type,
                         const QString &host = "http://52.59.243.90/", int port = 6755);
};

}//namespace connect


#endif // SERVERCONNECTOR_H
