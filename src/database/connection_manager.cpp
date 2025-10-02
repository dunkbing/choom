#include "connection_manager.h"
#include "sqlite_connection.h"
#include "mysql_connection.h"
#include "postgres_connection.h"

ConnectionManager& ConnectionManager::instance() {
    static ConnectionManager instance;
    return instance;
}

void ConnectionManager::addConnection(const ConnectionConfig &config) {
    auto connection = createConnection(config);
    if (connection) {
        connections.append(connection);
        emit connectionAdded(config.name);
    }
}

void ConnectionManager::removeConnection(const QString &name) {
    for (int i = 0; i < connections.size(); ++i) {
        if (connections[i]->getName() == name) {
            connections.removeAt(i);
            emit connectionRemoved(name);
            break;
        }
    }
}

DatabaseConnection* ConnectionManager::getConnection(const QString &name) {
    for (auto &conn : connections) {
        if (conn->getName() == name) {
            return conn.get();
        }
    }
    return nullptr;
}

QVector<DatabaseConnection*> ConnectionManager::getAllConnections() const {
    QVector<DatabaseConnection*> result;
    for (const auto &conn : connections) {
        result.append(conn.get());
    }
    return result;
}

std::shared_ptr<DatabaseConnection> ConnectionManager::createConnection(const ConnectionConfig &config) {
    switch (config.type) {
        case DatabaseType::SQLite:
            return std::make_shared<SQLiteConnection>(config);
        case DatabaseType::MySQL:
            return std::make_shared<MySQLConnection>(config);
        case DatabaseType::PostgreSQL:
            return std::make_shared<PostgresConnection>(config);
        default:
            return nullptr;
    }
}
