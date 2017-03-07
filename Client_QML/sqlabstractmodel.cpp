#include "sqlabstractmodel.h"
#include <QMutexLocker>
#include <QSqlRecord>
#include <QSqlError>
#include <QtConcurrent>
#include <QSqlQuery>
#include <serverconnector.h>



QString tableNameFor(QString str) {
    return str.replace("TOKEN", connect::ServerConnector::Instance()->getToken());
}

using namespace model;
SqlAbstractModel::SqlAbstractModel(QObject *parent, const QString &createTableQuery, const QString &tableName):
    QSqlTableModel{parent},
    createTableQuery_{createTableQuery},
    tableName_{tableName} {
    //init();
}

void SqlAbstractModel::init() {
    tableName_ = {tableNameFor(tableName_)};
    createTableQuery_ = {tableNameFor(createTableQuery_)};
    createTable(this);
}

void SqlAbstractModel::createTable(SqlAbstractModel *model) {
    //First check if table exists
    connect(&model->futureWatcher_, SIGNAL(finished()), model, SLOT(tableCreationFinished()));
    if (QSqlDatabase::database().tables().contains(model->tableName_)) {
        model->tableCreationFinished();
        return;
    }

    QSqlQuery query;
    if (!query.exec(model->createTableQuery_)) {
        qFatal("Failed to create database: %s", qPrintable(query.lastError().text()));
    }

    /*const auto fut = QtConcurrent::run([model] () {
        qDebug() << model->createTableQuery_;
        QMutexLocker lock{&model->tableMutex_};
        QSqlQuery query;
        if (!query.exec(model->createTableQuery_)) {
            qFatal("Failed to create database: %s", qPrintable(query.lastError().text()));
        }
    });
    model->futureWatcher_.setFuture(fut);*/
    model->tableCreationFinished();
}


void SqlAbstractModel::tableCreationFinished() {
    setTable(tableName_);
    futureWatcher_.disconnect(SIGNAL(finished()));
    connect(&futureWatcher_, SIGNAL(finished()), this, SLOT(modelWasSet()));
}

QVariant SqlAbstractModel::data(const QModelIndex &index, int role) const{
   // qDebug() << index;
    if (role < Qt::UserRole)
        return QSqlTableModel::data(index, role);
    const QSqlRecord sqlRecord = record(index.row());
    auto res = sqlRecord.value(QString(roleNames().value(role))).toString();
    return res;
}

void SqlAbstractModel::modelWasSet() {
    emit modelChanged();
}

