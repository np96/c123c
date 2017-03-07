#ifndef SQLGROUPMODEL_H
#define SQLGROUPMODEL_H
#include <QSqlTableModel>
#include "sqlabstractmodel.h"

class QJsonObject;

namespace model {

class SqlGroupModel : public SqlAbstractModel
{
    Q_OBJECT
    Q_PROPERTY(QString group READ group WRITE setGroup NOTIFY modelChanged)

public:
    ~SqlGroupModel() {
        qDebug() << "destroying model";
    }

    explicit SqlGroupModel(QObject *parent = 0);

    inline const QString& group() const noexcept {
        return group_;
    }

    void setGroup(const QString &group);

    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;


    Q_INVOKABLE void killSlots() {
        blockSignals(true);
    }

    Q_INVOKABLE void sendMessage(const QString& id, const QString &name);

    bool addMessage(const QString &group, const QString &content,
                    const QString &timestamp, const QString &login,
                    const QString &trueId = "");
public slots:
    void messageRecieved(const QJsonObject &message, const QString &id);
    void messagesReceived(const QJsonObject &messages);
    void messageDelivered(const QJsonObject &message);
private:
    QString group_;

    const QHash<int, QByteArray> roles =
                {{Qt::UserRole, "uid"},
                 {Qt::UserRole + 1, "sentby"},
                 {Qt::UserRole + 2, "groupid"},
                 {Qt::UserRole + 3, "timestamp"},
                 {Qt::UserRole + 4, "content"},
                 {Qt::UserRole + 5, "trueId"}
                };
signals:
    void modelChanged();


};
} //model

#endif // SQLGROUPMODEL_H
