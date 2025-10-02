#include "query_executor.h"
#include <QElapsedTimer>
#include <QSqlError>
#include <QThread>

QueryExecutor::QueryExecutor(QObject *parent)
    : QObject(parent) {
}

ConnectionInfo QueryExecutor::getConnectionInfo(const QString &connectionName) {
    ConnectionInfo info;
    QSqlDatabase db = QSqlDatabase::database(connectionName, false);
    if (db.isValid()) {
        info.driverName = db.driverName();
        info.databaseName = db.databaseName();
        info.hostName = db.hostName();
        info.userName = db.userName();
        info.password = db.password();
        info.port = db.port();
        info.connectOptions = db.connectOptions();
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

QFuture<QueryResult> QueryExecutor::executeTableQuery(const QString &connectionName, const QString &tableName, int limit, int offset) {
    // Quote table name to handle special characters and reserved words
    QString quotedTableName = QString("\"%1\"").arg(tableName);
    QString query = QString("SELECT * FROM %1 LIMIT %2 OFFSET %3")
                        .arg(quotedTableName)
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
