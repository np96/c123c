#ifndef SQLCONTACTMODEL_H
#define SQLCONTACTMODEL_H
#include "sqlabstractmodel.h"

namespace model {

class SqlContactModel : public SqlAbstractModel
{
    Q_OBJECT
    Q_PROPERTY(QString contact READ contact WRITE setContact NOTIFY contactChanged)
public:
    explicit SqlContactModel(QObject *parent = 0);

    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;

    Q_INVOKABLE void addContact(const QString& id, const QString& type);


    void setContact(const QString &contact, bool disconnect = true);

    const QString &contact() const {
        return contact_;
    }

    Q_INVOKABLE void addMember(const QString& memberId, const QString &groupId);

    Q_INVOKABLE void removeMember(const QString &member, const QString &group);

public slots:
    void proceedNewGroups(const QJsonArray &groups);
    void proceedGroupMembers(const QJsonArray &members, const QString &groupId);
    void proceedGroupAdmins(const QString &admin, const QString &group);

    bool addMemberToTable(const QString &memberId, const QString &groupId);
    bool removeMemberFromTable(const QString &memberId, const QString &groupId);
signals:
    void contactChanged();


private:



    QString contact_ = "undef";

    const QHash<int, QByteArray> roles = {
        {Qt::UserRole, "contactid"},
        {Qt::UserRole + 1, "type"},
        {Qt::UserRole + 2, "name"},
        {Qt::UserRole + 3, "members"},
        {Qt::UserRole + 4, "lastmessage"},
        {Qt::UserRole + 5, "admin"}
    };
};

} //model

#endif // SQLCONTACTMODEL_H
