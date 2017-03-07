#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSqlDatabase>
#include <QSqlError>
#include "serverconnector.h"
#include "sqlcontactmodel.h"
#include "sqlgroupmodel.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <serverconnector.h>

static void connectToDatabase()
{
    QSqlDatabase database = QSqlDatabase::database();
    if (!database.isValid()) {
        database = QSqlDatabase::addDatabase("QSQLITE");
        if (!database.isValid())
            qFatal("Cannot add database: %s", qPrintable(database.lastError().text()));
    }

    const QDir writeDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!writeDir.mkpath("."))
        qFatal("Failed to create writable directory at %s", qPrintable(writeDir.absolutePath()));

    // Ensure that we have a writable location on all devices.
    const QString fileName = writeDir.absolutePath() + "/qmlchat-database_62.sqlite3";
    // When using the SQLite driver, open() will create the SQLite database if it doesn't exist.
    database.setDatabaseName(fileName);
    if (!database.open()) {
        qFatal("Cannot open database: %s", qPrintable(database.lastError().text()));
        QFile::remove(fileName);
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    connect::ServerConnector::Instance()->setToken("null");
    connectToDatabase();
    qmlRegisterType<model::SqlContactModel>("cpp.chat.qml", 1, 0, "SqlContactModel");
    qmlRegisterType<model::SqlGroupModel>("cpp.chat.qml", 1, 0, "SqlGroupModel");

    engine.rootContext()->setContextProperty("serverConnect", connect::ServerConnector::Instance());
    engine.rootContext()->setContextProperty("methods", connect::ServerConnector::Instance());
    engine.load(QUrl(QLatin1String("qrc:/main.qml")));


    return app.exec();
}
