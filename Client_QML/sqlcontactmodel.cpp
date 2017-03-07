#include "sqlcontactmodel.h"
#include "assert.h"
#include "serverconnector.h"
#include "QSqlRecord"
#include <QSqlError>
#include <QSqlField>
#include <QStringList>
#include <QSqlQuery>

static const QString createTableQuery = "CREATE TABLE IF NOT EXISTS 'ContactsTOKEN' ("
                                      "'contactid' int NOT NULL,"
                                      "'type' TEXT NOT NULL,"
                                      "'name' TEXT NOT NULL,"
                                      "'members' TEXT NOT NULL,"
                                      "'lastmessage' TEXT,"
                                      "'admin' TEXT, "
                                      "PRIMARY KEY ('contactid'))";

static const QString sqlTableName = "ContactsTOKEN";

using namespace model;
using namespace connect;

QSqlQuery addMemberQuery;
SqlContactModel::SqlContactModel(QObject *parent):
    SqlAbstractModel(parent, createTableQuery, sqlTableName) {
    connect(ServerConnector::Instance(), &ServerConnector::newGroups,
            this, &SqlContactModel::proceedNewGroups);
    connect(ServerConnector::Instance(), &ServerConnector::receivedGroupMembers,
            this, &SqlContactModel::proceedGroupMembers);
    connect(ServerConnector::Instance(), &ServerConnector::receivedGroupAdmins,
            this, &SqlContactModel::proceedGroupAdmins);
    setSort(1, Qt::DescendingOrder);
    setEditStrategy(QSqlTableModel::OnManualSubmit);
}




void SqlContactModel::addContact(const QString &id, const QString &type) {
    QString login = connect::ServerConnector::Instance()->settings_.value("login","").toString();
    assert(login != "");
    QSqlQuery query;
    QString queryString = "INSERT OR IGNORE INTO ContactsTOKEN (contactid, type, name, members)"
                          " VALUES (:contactid, :type, :name, :members)";
    queryString = queryString.replace("ContactsTOKEN", tableName_);
    if (!query.prepare(queryString)) {
        qWarning() << lastError().text();
    }
    query.bindValue(0, id);
    query.bindValue(1, type);
    query.bindValue(2, id);
    query.bindValue(3, login);
    qDebug() << query.lastQuery();
    if (!query.exec()) {
        qDebug() << query.lastQuery();
        qWarning() << query.lastError();
    }
    submitAll();
    select();
    qDebug()<<"ROW COUNT " << rowCount();
}

void SqlContactModel::setContact(const QString &contact, bool disconnect) {
    if (contact_ == contact) return;
    init();
    if (contact != "" && disconnect) {
        ServerConnector::Instance()->disconnect(this);
        connect(ServerConnector::Instance(), &ServerConnector::memberAdded,
                this, &SqlContactModel::addMemberToTable);
        connect(ServerConnector::Instance(), &ServerConnector::memberRemoved,
                this, &SqlContactModel::removeMemberFromTable);
    } else {
        if (contact == "")
            ServerConnector::Instance()->getUpdatesRequest(-1, -1);
    }
    contact_ = contact;
    const auto& filterString = contact!="" ? QString::fromLatin1("contactid = %1").arg(contact_) : "";
    setFilter(filterString);
    if (!select()) {
        qDebug() << "error SELECT " <<lastError().text();
    }
}
bool SqlContactModel::addMemberToTable(const QString &member, const QString &group) {
    auto rec = record(0);
    if (group != rec.field("contactid").value().toString()) return false;
    auto field = rec.field("members");
    assert(!field.value().isNull());
    const auto &membersRaw = field.value().toString();
    qDebug() << membersRaw << " " << group;
    if (!membersRaw.split(",").contains(member)) {
        rec.setValue("members", membersRaw + "," + member);
        setRecord(0, rec);
        submitAll();
        return true;
    }
    return false;
}

bool SqlContactModel::removeMemberFromTable(const QString &member, const QString& group) {
    auto rec = record(0);
    if (group != rec.field("contactid").value().toString()) return false;
    auto field = rec.field("members");
    assert(!field.value().isNull());
    const auto &membersRaw = field.value().toString();
    qDebug() << membersRaw << " " << group;
    QStringList members = membersRaw.split(",");
    if (members.removeOne(member)) {
        rec.setValue("members", members.join(","));
        setRecord(0, rec);
        submitAll();
        return true;
    }
    return false;
}

void SqlContactModel::removeMember(const QString &member, const QString &group) {
    qDebug() << "removing " << member;
    ServerConnector::Instance()->addMemberRequest(member, group, "remove");
}

void SqlContactModel::addMember(const QString &member, const QString &group) {
    qDebug() << "adding " << member;
    ServerConnector::Instance()->addMemberRequest(member, group);
}

void SqlContactModel::proceedNewGroups(const QJsonArray &groups) {
    QMutexLocker lock{&tableMutex_};
    for (const auto &group: groups) {
        this->addContact(group.toString(), "group");
        ServerConnector::Instance()->getGroupMembersRequest(group.toString());
        submitAll();
    }
    setFilter("");
    submitAll();
    select();
    emit modelChanged();
}

void SqlContactModel::proceedGroupMembers(const QJsonArray &members, const QString &groupId) {
    QMutexLocker lock{&tableMutex_};
    setContact(groupId, false);
    for (const auto &member: members) {
        this->addMemberToTable(member.toString(), groupId);
    }
    submitAll();
    setFilter("");
    select();
}


//currently only 1 admin supported
void SqlContactModel::proceedGroupAdmins(const QString &admin, const QString &group) {
    QMutexLocker lock{&tableMutex_};
    setFilter(QString::fromLatin1("contactid = %1").arg(group));
    auto rec = record(0);
    if (group == rec.field("contactid").value().toString())  {
        rec.setValue("admin", admin);
        setRecord(0, rec);
    }
    submitAll();
    qDebug() << record(0);
    setFilter("");
    select();
}

QHash<int, QByteArray> SqlContactModel::roleNames() const {
    return roles;
}
