#include "database/postgres_connection.h"
#include <QSqlQuery>

PostgresConnection::PostgresConnection(const ConnectionConfig &config, QObject *parent)
    : DatabaseConnection(config, parent) {
}

bool PostgresConnection::connect() {
    db = QSqlDatabase::addDatabase("QPSQL", generateConnectionName());
    db.setHostName(config.host);
    db.setPort(config.port);
    db.setDatabaseName(config.database.isEmpty() ? "postgres" : config.database);
    db.setUserName(config.username);
    db.setPassword(config.password);

    if (!db.open()) {
        lastError = db.lastError().text();
        emit errorOccurred(lastError);
        return false;
    }

    emit connected();
    return true;
}

void PostgresConnection::disconnect() {
    if (db.isOpen()) {
        db.close();
        emit disconnected();
    }
}

bool PostgresConnection::isConnected() const {
    return db.isOpen();
}

QStringList PostgresConnection::getDatabases() {
    QStringList databases;
    QSqlQuery query(db);
    query.exec("SELECT datname FROM pg_database WHERE datistemplate = false ORDER BY datname");

    while (query.next()) {
        databases << query.value(0).toString();
    }

    return databases;
}

QStringList PostgresConnection::getSchemas(const QString &database) {
    Q_UNUSED(database);

    QStringList schemas;
    QSqlQuery query(db);
    query.exec(
        "SELECT schema_name FROM information_schema.schemata "
        "WHERE schema_name NOT LIKE 'pg_%' AND schema_name != 'information_schema' "
        "ORDER BY schema_name"
    );

    while (query.next()) {
        schemas << query.value(0).toString();
    }

    return schemas;
}

QStringList PostgresConnection::getTables(const QString &schema, const QString &database) {
    Q_UNUSED(database);

    QStringList tables;
    QSqlQuery query(db);

    QString sql = "SELECT tablename FROM pg_tables";
    if (!schema.isEmpty()) {
        sql += QString(" WHERE schemaname = '%1'").arg(schema);
    } else {
        sql += " WHERE schemaname NOT IN ('pg_catalog', 'information_schema')";
    }
    sql += " ORDER BY tablename";

    query.exec(sql);

    while (query.next()) {
        tables << query.value(0).toString();
    }

    return tables;
}

QStringList PostgresConnection::getViews(const QString &schema, const QString &database) {
    Q_UNUSED(database);

    QStringList views;
    QSqlQuery query(db);

    QString sql = "SELECT viewname FROM pg_views";
    if (!schema.isEmpty()) {
        sql += QString(" WHERE schemaname = '%1'").arg(schema);
    } else {
        sql += " WHERE schemaname NOT IN ('pg_catalog', 'information_schema')";
    }
    sql += " ORDER BY viewname";

    query.exec(sql);

    while (query.next()) {
        views << query.value(0).toString();
    }

    return views;
}

QStringList PostgresConnection::getSequences(const QString &schema, const QString &database) {
    Q_UNUSED(database);

    QStringList sequences;
    QSqlQuery query(db);

    QString sql =
        "SELECT sequence_name FROM information_schema.sequences";
    if (!schema.isEmpty()) {
        sql += QString(" WHERE sequence_schema = '%1'").arg(schema);
    } else {
        sql += " WHERE sequence_schema NOT IN ('pg_catalog', 'information_schema')";
    }
    sql += " ORDER BY sequence_name";

    query.exec(sql);

    while (query.next()) {
        sequences << query.value(0).toString();
    }

    return sequences;
}
