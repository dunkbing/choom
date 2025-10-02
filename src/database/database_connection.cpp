#include "database/database_connection.h"
#include <QUuid>

DatabaseConnection::DatabaseConnection(const ConnectionConfig &config, QObject *parent)
    : QObject(parent), config(config) {
}

DatabaseConnection::~DatabaseConnection() {
    if (db.isOpen()) {
        db.close();
    }
    QSqlDatabase::removeDatabase(db.connectionName());
}

QString DatabaseConnection::generateConnectionName() const {
    return config.name + "_" + QUuid::createUuid().toString();
}
