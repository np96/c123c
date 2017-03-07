#ifndef SQLABSTRACTMODEL_H
#define SQLABSTRACTMODEL_H
#include <QSqlTableModel>
#include <QFutureWatcher>
#include <QMutex>
#include <QSqlRelationalTableModel>
namespace model {

class SqlAbstractModel : public QSqlTableModel
{
    Q_OBJECT

public:

    static void createTable(SqlAbstractModel *model);

    SqlAbstractModel(QObject *parent = 0, const QString& createTableQuery = "", const QString &tableName = "");

    Q_INVOKABLE QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;

    Q_INVOKABLE virtual void init();

public slots:

    virtual void modelWasSet();

    void tableCreationFinished();

signals:
    void modelChanged();


protected:
    mutable QMutex tableMutex_;
    mutable QFutureWatcher<void> futureWatcher_;
    QString tableName_;

private:
    QString createTableQuery_;

};
} //model

#endif // SQLABSTRACTMODEL_H
