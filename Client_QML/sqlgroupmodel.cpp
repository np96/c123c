#include "sqlgroupmodel.h"
#include "serverconnector.h"
#include <QDateTime>
#include <QDebug>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QDateTime>
#include <QtConcurrent/QtConcurrent>
#include <assert.h>

using namespace connect;
using namespace model;
static const QString groupsTableName = "MessagesTOKEN";

static const QString createTableQuery = "CREATE TABLE IF NOT EXISTS 'MessagesTOKEN' ("
                                      "'uid' INTEGER PRIMARY KEY,"
                                      "'sentby' TEXT NOT NULL,"
                                      "'groupid' TEXT NOT NULL,"
                                      "'timestamp' TEXT NOT NULL,"
                                      "'content' TEXT NOT NULL,"
                                      "'trueid' TEXT UNIQUE,"
                                      //"FOREIGN KEY(sentby) REFERENCES Contacts ( id ),"
                                      "FOREIGN KEY(groupid) REFERENCES Contacts ( id ))";


SqlGroupModel::SqlGroupModel(QObject *parent):
   SqlAbstractModel(parent, createTableQuery, groupsTableName) {
    setSort(3, Qt::AscendingOrder);
    setEditStrategy(QSqlTableModel::OnManualSubmit);
    connect(ServerConnector::Instance(), &ServerConnector::messageAccepted, this, &SqlGroupModel::messageDelivered);
    connect(ServerConnector::Instance(), &ServerConnector::newMessage, this, &SqlGroupModel::messageRecieved);
    connect(ServerConnector::Instance(), &ServerConnector::messages, this, &SqlGroupModel::messagesReceived);
}


void SqlGroupModel::setGroup(const QString &group)  {
    init();
    if (group != group_) {
        group_ = group;
    }
    const auto& filterString = QString::fromLatin1("groupid = %1").arg(group_);
    setFilter(filterString);
    if (!select()) {
        qDebug() << "error SELECT " <<lastError().text();
    }
    ServerConnector::Instance()->getUpdatesRequest(group.toInt(), rowCount());

    emit modelChanged();

    /*auto fut = QtConcurrent::run([this] {
        QMutexLocker lock{&tableMutex_};
        if (!select()) {
            qDebug() <<lastError().text();
        }
    });/
    futureWatcher_.setFuture(fut);*/
}
QString unixTimeToString(const QString& unixTime) {
    bool ok;
    const uint s = unixTime.left(10).toUInt(&ok);
    assert(ok);
    const QDateTime &dt = QDateTime::fromTime_t(s);
    return dt.toString(Qt::ISODate);
}

QHash<int, QByteArray> SqlGroupModel::roleNames() const {
    return roles;
}

void SqlGroupModel::messageRecieved(const QJsonObject &message, const QString &id) {
    if (addMessage(message["group"].toString(), message["content"].toString(),
            unixTimeToString(message["timestamp"].toString()),
            message["author"].toString(), id)) {
    //    ServerConnector::Instance()->getUpdatesRequest(message["group"].toString().toInt(), rowCount());
    }
}

void SqlGroupModel::messagesReceived(const QJsonObject &messages) {
    bool needMore = true;
    for (const auto &elem : messages.keys()) {
        if (needMore) {
          ServerConnector::Instance()->getUpdatesRequest(
            messages[elem].toObject()["group"].toString().toInt(), rowCount() + messages.keys().length());
          needMore = false;
        }
        messageRecieved(messages[elem].toObject(), elem);
    }
}

void SqlGroupModel::messageDelivered(const QJsonObject &message) {
    QString queryString = "UPDATE MessagesTOKEN SET trueid=:trueid WHERE uid=:uid";
    queryString = queryString.replace("MessagesTOKEN", tableName());
    QSqlQuery query;
    if (!query.prepare(queryString)) {
        qWarning() << lastError().text();
    }
    auto key = message.keys();
    query.bindValue(0, key[0]);
    query.bindValue(1, message.value(key[0]));
    query.exec();
    selectRow(key[0].toInt());
}

bool SqlGroupModel::addMessage(const QString &group, const QString &content,
                               const QString &timestamp, const QString &login,
                               const QString &trueId) {
    QString queryString =
        QString::fromLatin1("INSERT OR IGNORE INTO %1 (sentby, groupid, timestamp, content, trueid)"
                           " VALUES (:sentby, :groupid, :timestamp, :content, :trueid)")
                            .arg(tableName_);
    assert(login != "");
    assert(timestamp != "");
    assert(group != "");
    QSqlQuery query;
    if (!query.prepare(queryString)) {
        qWarning() << lastError().text();
    }
    query.bindValue(0, login);
    query.bindValue(1, group);
    query.bindValue(2, timestamp);
    query.bindValue(3, content);
    query.bindValue(4, trueId);
    auto res = query.exec();
    if (!res) {
        qDebug() << query.lastQuery();
        qWarning() << query.lastError();
    } else {
        submitAll();
        select();
        emit modelChanged();
    }
    return res;
}

void SqlGroupModel::sendMessage(const QString &group, const QString &content) {
    const QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
    QString login = connect::ServerConnector::Instance()->settings_.value("login","").toString();
    if (addMessage(group, content, timestamp, login, timestamp))
        ServerConnector::Instance()->sendMessageRequest(group, content,
                                                    record(rowCount() - 1).value("uid").toInt());
    //qDebug() << record(rowCount() - 1);
}
