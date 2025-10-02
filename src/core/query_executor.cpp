#include "query_executor.h"
#include <QElapsedTimer>
#include <QSqlError>

QueryExecutor::QueryExecutor(QObject *parent)
    : QObject(parent) {
}

QFuture<QueryResult> QueryExecutor::executeQuery(const QString &connectionName, const QString &query) {
    emit queryStarted();
    return QtConcurrent::run([this, connectionName, query]() {
        auto result = runQuery(connectionName, query);
        emit queryFinished(result);
        if (!result.success) {
            emit queryError(result.errorMessage);
        }
        return result;
    });
}

QFuture<QueryResult> QueryExecutor::executeTableQuery(const QString &connectionName, const QString &tableName, int limit, int offset) {
    QString query = QString("SELECT * FROM %1 LIMIT %2 OFFSET %3")
                        .arg(tableName)
                        .arg(limit)
                        .arg(offset);
    return executeQuery(connectionName, query);
}

QueryResult QueryExecutor::runQuery(const QString &connectionName, const QString &query) {
    QueryResult result;
    result.success = false;
    result.rowCount = 0;

    QElapsedTimer timer;
    timer.start();

    // Get the database connection
    QSqlDatabase db = QSqlDatabase::database(connectionName);
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
