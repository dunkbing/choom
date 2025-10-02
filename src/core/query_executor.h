#ifndef QUERY_EXECUTOR_H
#define QUERY_EXECUTOR_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariantList>
#include <QFuture>
#include <QtConcurrent>

struct ConnectionInfo {
    QString driverName;
    QString databaseName;
    QString hostName;
    QString userName;
    QString password;
    int port;
    QString connectOptions;
};

struct QueryResult {
    bool success;
    QString errorMessage;
    QList<QSqlRecord> records;
    QStringList columnNames;
    int rowCount;
    qint64 executionTimeMs;
};

class QueryExecutor : public QObject {
    Q_OBJECT

public:
    explicit QueryExecutor(QObject *parent = nullptr);

    // Async query execution
    QFuture<QueryResult> executeQuery(const QString &connectionName, const QString &query);
    QFuture<QueryResult> executeTableQuery(const QString &connectionName, const QString &tableName,
                                           const QString &databaseName = QString(), const QString &schemaName = QString(),
                                           int limit = 1000, int offset = 0);

signals:
    void queryStarted();
    void queryFinished(const QueryResult &result);
    void queryError(const QString &error);

private:
    static ConnectionInfo getConnectionInfo(const QString &connectionName);
    static QueryResult runQuery(const ConnectionInfo &connInfo, const QString &query);
};

#endif // QUERY_EXECUTOR_H
