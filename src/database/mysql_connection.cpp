#include "database/mysql_connection.h"
#include <QSqlQuery>

MySQLConnection::MySQLConnection(const ConnectionConfig &config, QObject *parent)
    : DatabaseConnection(config, parent) {
}

bool MySQLConnection::connect() {
    db = QSqlDatabase::addDatabase("QMYSQL", generateConnectionName());
    db.setHostName(config.host);
    db.setPort(config.port);
    db.setUserName(config.username);
    db.setPassword(config.password);

    if (!config.database.isEmpty()) {
        db.setDatabaseName(config.database);
    }

    if (!db.open()) {
        lastError = db.lastError().text();
        emit errorOccurred(lastError);
        return false;
    }

    emit connected();
    return true;
}

void MySQLConnection::disconnect() {
    if (db.isOpen()) {
        db.close();
        emit disconnected();
    }
}

bool MySQLConnection::isConnected() const {
    return db.isOpen();
}

QStringList MySQLConnection::getDatabases() {
    QStringList databases;
    QSqlQuery query(db);
    query.exec("SHOW DATABASES");

    while (query.next()) {
        QString dbName = query.value(0).toString();
        // Filter out system databases
        if (dbName != "information_schema" && dbName != "performance_schema" &&
            dbName != "mysql" && dbName != "sys") {
            databases << dbName;
        }
    }

    return databases;
}

QStringList MySQLConnection::getSchemas(const QString &database) {
    // In MySQL, schemas and databases are the same
    Q_UNUSED(database);
    return getDatabases();
}

QStringList MySQLConnection::getTables(const QString &schema, const QString &database) {
    QString dbName = database.isEmpty() ? schema : database;

    QStringList tables;
    QSqlQuery query(db);

    if (dbName.isEmpty()) {
        query.exec("SHOW TABLES");
    } else {
        query.exec(QString("SHOW TABLES FROM `%1`").arg(dbName));
    }

    while (query.next()) {
        tables << query.value(0).toString();
    }

    return tables;
}

QStringList MySQLConnection::getViews(const QString &schema, const QString &database) {
    QString dbName = database.isEmpty() ? schema : database;

    QStringList views;
    QSqlQuery query(db);

    QString sql = QString(
        "SELECT TABLE_NAME FROM information_schema.TABLES "
        "WHERE TABLE_TYPE = 'VIEW'"
    );

    if (!dbName.isEmpty()) {
        sql += QString(" AND TABLE_SCHEMA = '%1'").arg(dbName);
    }

    query.exec(sql);

    while (query.next()) {
        views << query.value(0).toString();
    }

    return views;
}

QStringList MySQLConnection::getSequences(const QString &schema, const QString &database) {
    Q_UNUSED(schema);
    Q_UNUSED(database);

    // MySQL doesn't have sequences (uses AUTO_INCREMENT)
    return QStringList();
}
