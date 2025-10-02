#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include "database_connection.h"
#include <QObject>
#include <QVector>
#include <memory>

class ConnectionManager : public QObject {
    Q_OBJECT

public:
    static ConnectionManager& instance();

    void addConnection(const ConnectionConfig &config);
    void removeConnection(const QString &name);
    DatabaseConnection* getConnection(const QString &name);
    QVector<DatabaseConnection*> getAllConnections() const;

    std::shared_ptr<DatabaseConnection> createConnection(const ConnectionConfig &config);

signals:
    void connectionAdded(const QString &name);
    void connectionRemoved(const QString &name);

private:
    ConnectionManager() = default;
    ~ConnectionManager() = default;
    ConnectionManager(const ConnectionManager&) = delete;
    ConnectionManager& operator=(const ConnectionManager&) = delete;

    QVector<std::shared_ptr<DatabaseConnection>> connections;
};

#endif // CONNECTION_MANAGER_H
