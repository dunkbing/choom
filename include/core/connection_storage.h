#ifndef CONNECTION_STORAGE_H
#define CONNECTION_STORAGE_H

#include <QString>
#include <QVector>
#include <QSqlDatabase>
#include "database/database_connection.h"

class ConnectionStorage {
public:
    static ConnectionStorage& instance();

    bool initialize();
    bool saveConnection(const ConnectionConfig &config);
    bool removeConnection(const QString &name);
    QVector<ConnectionConfig> loadAllConnections();
    bool updateConnection(const QString &oldName, const ConnectionConfig &config);

private:
    ConnectionStorage() = default;
    ~ConnectionStorage() = default;
    ConnectionStorage(const ConnectionStorage&) = delete;
    ConnectionStorage& operator=(const ConnectionStorage&) = delete;

    bool createTables();
    QString encryptPassword(const QString &password) const;
    QString decryptPassword(const QString &encrypted) const;

    QSqlDatabase storageDb;
    const QString STORAGE_DB_NAME = "connections_storage";
};

#endif // CONNECTION_STORAGE_H
