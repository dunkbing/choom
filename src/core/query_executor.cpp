#include "core/query_executor.h"
#include "database/connection_manager.h"
#include <QElapsedTimer>
#include <QSqlError>
#include <QThread>
#include <QDebug>

QueryExecutor::QueryExecutor(QObject *parent)
    : QObject(parent) {
}

ConnectionInfo QueryExecutor::getConnectionInfo(const QString &connectionName) {
    ConnectionInfo info;

    // Get the actual database connection from manager
    DatabaseConnection *conn = ConnectionManager::instance().getConnection(connectionName);
    if (conn && conn->isConnected()) {
        ConnectionConfig config = conn->getConfig();

        qDebug() << "Getting connection info for:" << connectionName << "Type:" << (int)config.type;

        // Map database type to driver name
        switch (config.type) {
            case DatabaseType::SQLite:
                info.driverName = "QSQLITE";
                info.databaseName = config.filePath;
                break;
            case DatabaseType::MySQL:
                info.driverName = "QMYSQL";
                info.databaseName = config.database;
                info.hostName = config.host;
                info.port = config.port;
                info.userName = config.username;
                info.password = config.password;
                break;
            case DatabaseType::PostgreSQL:
                info.driverName = "QPSQL";
                info.databaseName = config.database;
                info.hostName = config.host;
                info.port = config.port;
                info.userName = config.username;
                info.password = config.password;
                break;
        }
        qDebug() << "Connection info - Driver:" << info.driverName << "DB:" << info.databaseName;
    } else {
        qDebug() << "Connection not found or not connected:" << connectionName;
    }
    return info;
}

QFuture<QueryResult> QueryExecutor::executeQuery(const QString &connectionName, const QString &query) {
    emit queryStarted();

    // Get connection info in main thread
    ConnectionInfo connInfo = getConnectionInfo(connectionName);

    return QtConcurrent::run([this, connInfo, query]() {
        auto result = runQuery(connInfo, query);
        emit queryFinished(result);
        if (!result.success) {
            emit queryError(result.errorMessage);
        }
        return result;
    });
}

QFuture<QueryResult> QueryExecutor::executeTableQuery(const QString &connectionName, const QString &tableName,
                                                       const QString &databaseName, const QString &schemaName,
                                                       int limit, int offset) {
    // Build fully qualified table name
    QString qualifiedTableName;

    // For PostgreSQL/MySQL with schema, table name may already be qualified (schema.table)
    if (tableName.contains('.')) {
        // Already qualified, just quote each part
        QStringList parts = tableName.split('.');
        for (QString &part : parts) {
            part = QString("\"%1\"").arg(part);
        }
        qualifiedTableName = parts.join('.');
    } else {
        // Quote table name
        qualifiedTableName = QString("\"%1\"").arg(tableName);
    }

    QString query = QString("SELECT * FROM %1 LIMIT %2 OFFSET %3")
                        .arg(qualifiedTableName)
                        .arg(limit)
                        .arg(offset);
    return executeQuery(connectionName, query);
}

QueryResult QueryExecutor::runQuery(const ConnectionInfo &connInfo, const QString &query) {
    QueryResult result;
    result.success = false;
    result.rowCount = 0;

    QElapsedTimer timer;
    timer.start();

    // Create a thread-specific connection
    QString threadConnectionName = QString("thread_%1").arg((quintptr)QThread::currentThreadId());

    QSqlDatabase db;
    if (QSqlDatabase::contains(threadConnectionName)) {
        db = QSqlDatabase::database(threadConnectionName, false);
    } else {
        // Create new connection for this thread
        db = QSqlDatabase::addDatabase(connInfo.driverName, threadConnectionName);
        db.setDatabaseName(connInfo.databaseName);
        db.setHostName(connInfo.hostName);
        db.setUserName(connInfo.userName);
        db.setPassword(connInfo.password);
        db.setPort(connInfo.port);
        db.setConnectOptions(connInfo.connectOptions);

        if (!db.open()) {
            result.errorMessage = "Failed to open database connection: " + db.lastError().text();
            return result;
        }
    }

    if (!db.isValid() || !db.isOpen()) {
        result.errorMessage = "Database connection is not valid or not open";
        return result;
    }

    QSqlQuery sqlQuery(db);
    if (!sqlQuery.exec(query)) {
        result.errorMessage = sqlQuery.lastError().text();
        result.executionTimeMs = timer.elapsed();
        return result;
    }

    // Get column names
    QSqlRecord record = sqlQuery.record();
    for (int i = 0; i < record.count(); ++i) {
        result.columnNames << record.fieldName(i);
    }

    // Fetch all results
    while (sqlQuery.next()) {
        result.records.append(sqlQuery.record());
        result.rowCount++;
    }

    result.success = true;
    result.executionTimeMs = timer.elapsed();

    return result;
}
