#ifndef POSTGRES_CONNECTION_H
#define POSTGRES_CONNECTION_H

#include "database_connection.h"

class PostgresConnection : public DatabaseConnection {
    Q_OBJECT

public:
    explicit PostgresConnection(const ConnectionConfig &config, QObject *parent = nullptr);

    bool connect() override;
    void disconnect() override;
    bool isConnected() const override;

    QStringList getDatabases() override;
    QStringList getSchemas(const QString &database = QString()) override;
    QStringList getTables(const QString &schema = QString(), const QString &database = QString()) override;
    QStringList getViews(const QString &schema = QString(), const QString &database = QString()) override;
    QStringList getSequences(const QString &schema = QString(), const QString &database = QString()) override;
};

#endif // POSTGRES_CONNECTION_H
