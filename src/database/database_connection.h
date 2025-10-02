#ifndef DATABASE_CONNECTION_H
#define DATABASE_CONNECTION_H

#include <QObject>
#include <QString>
#include <QSqlDatabase>
#include <QSqlError>
#include <QStringList>
#include <memory>

enum class DatabaseType {
    SQLite,
    MySQL,
    PostgreSQL
};

struct ConnectionConfig {
    QString name;
    DatabaseType type;
    QString host;
    int port;
    QString database;
    QString username;
    QString password;
    QString filePath; // For SQLite
};

struct SchemaInfo {
    QString name;
    QStringList tables;
    QStringList views;
    QStringList sequences;
};

class DatabaseConnection : public QObject {
    Q_OBJECT

public:
    explicit DatabaseConnection(const ConnectionConfig &config, QObject *parent = nullptr);
    virtual ~DatabaseConnection();

    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;

    virtual QStringList getDatabases() = 0;
    virtual QStringList getSchemas(const QString &database = QString()) = 0;
    virtual QStringList getTables(const QString &schema = QString(), const QString &database = QString()) = 0;
    virtual QStringList getViews(const QString &schema = QString(), const QString &database = QString()) = 0;
    virtual QStringList getSequences(const QString &schema = QString(), const QString &database = QString()) = 0;

    QString getName() const { return config.name; }
    DatabaseType getType() const { return config.type; }
    QString getLastError() const { return lastError; }
    QString getConnectionName() const { return db.connectionName(); }
    QSqlDatabase getDatabase() const { return db; }
    ConnectionConfig getConfig() const { return config; }

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString &error);

protected:
    ConnectionConfig config;
    QSqlDatabase db;
    QString lastError;

    QString generateConnectionName() const;
};

#endif // DATABASE_CONNECTION_H
