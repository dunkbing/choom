#include "sqlite_connection.h"
#include <QSqlQuery>

SQLiteConnection::SQLiteConnection(const ConnectionConfig &config, QObject *parent)
    : DatabaseConnection(config, parent) {
}

bool SQLiteConnection::connect() {
    db = QSqlDatabase::addDatabase("QSQLITE", generateConnectionName());
    db.setDatabaseName(config.filePath);

    if (!db.open()) {
        lastError = db.lastError().text();
        emit errorOccurred(lastError);
        return false;
    }

    emit connected();
    return true;
}

void SQLiteConnection::disconnect() {
    if (db.isOpen()) {
        db.close();
        emit disconnected();
    }
}

bool SQLiteConnection::isConnected() const {
    return db.isOpen();
}

QStringList SQLiteConnection::getDatabases() {
    // SQLite has only one database per file
    return QStringList() << config.filePath;
}

QStringList SQLiteConnection::getSchemas(const QString &database) {
    // SQLite doesn't have schemas
    Q_UNUSED(database);
    return QStringList();
}

QStringList SQLiteConnection::getTables(const QString &schema, const QString &database) {
    Q_UNUSED(schema);
    Q_UNUSED(database);

    QStringList tables;
    QSqlQuery query(db);
    query.exec("SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%' ORDER BY name");

    while (query.next()) {
        tables << query.value(0).toString();
    }

    return tables;
}

QStringList SQLiteConnection::getViews(const QString &schema, const QString &database) {
    Q_UNUSED(schema);
    Q_UNUSED(database);

    QStringList views;
    QSqlQuery query(db);
    query.exec("SELECT name FROM sqlite_master WHERE type='view' ORDER BY name");

    while (query.next()) {
        views << query.value(0).toString();
    }

    return views;
}

QStringList SQLiteConnection::getSequences(const QString &schema, const QString &database) {
    Q_UNUSED(schema);
    Q_UNUSED(database);

    // SQLite doesn't have sequences in the traditional sense
    return QStringList();
}
