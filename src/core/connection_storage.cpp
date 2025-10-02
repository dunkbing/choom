#include "connection_storage.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QCryptographicHash>
#include <QByteArray>
#include <QSysInfo>

ConnectionStorage& ConnectionStorage::instance() {
    static ConnectionStorage instance;
    return instance;
}

bool ConnectionStorage::initialize() {
    // Get application data directory
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataDir);
    if (!dir.exists()) {
        dir.mkpath(dataDir);
    }

    QString dbPath = dataDir + "/connections.db";
    qDebug() << "Storage database path:" << dbPath;

    storageDb = QSqlDatabase::addDatabase("QSQLITE", STORAGE_DB_NAME);
    storageDb.setDatabaseName(dbPath);

    if (!storageDb.open()) {
        qWarning() << "Failed to open storage database:" << storageDb.lastError().text();
        return false;
    }

    return createTables();
}

bool ConnectionStorage::createTables() {
    QSqlQuery query(storageDb);

    QString createTableSQL = R"(
        CREATE TABLE IF NOT EXISTS connections (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT UNIQUE NOT NULL,
            type INTEGER NOT NULL,
            host TEXT,
            port INTEGER,
            database_name TEXT,
            username TEXT,
            password TEXT,
            file_path TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";

    if (!query.exec(createTableSQL)) {
        qWarning() << "Failed to create connections table:" << query.lastError().text();
        return false;
    }

    return true;
}

QString ConnectionStorage::encryptPassword(const QString &password) const {
    if (password.isEmpty()) {
        return QString();
    }

    // Simple XOR encryption with a key derived from machine ID
    // For production, consider using Qt's QCA or OpenSSL for stronger encryption
    QByteArray key = QCryptographicHash::hash(
        QSysInfo::machineUniqueId(),
        QCryptographicHash::Sha256
    );

    QByteArray passwordBytes = password.toUtf8();
    QByteArray encrypted;

    for (int i = 0; i < passwordBytes.size(); ++i) {
        encrypted.append(passwordBytes[i] ^ key[i % key.size()]);
    }

    return QString::fromLatin1(encrypted.toBase64());
}

QString ConnectionStorage::decryptPassword(const QString &encrypted) const {
    if (encrypted.isEmpty()) {
        return QString();
    }

    QByteArray key = QCryptographicHash::hash(
        QSysInfo::machineUniqueId(),
        QCryptographicHash::Sha256
    );

    QByteArray encryptedBytes = QByteArray::fromBase64(encrypted.toLatin1());
    QByteArray decrypted;

    for (int i = 0; i < encryptedBytes.size(); ++i) {
        decrypted.append(encryptedBytes[i] ^ key[i % key.size()]);
    }

    return QString::fromUtf8(decrypted);
}

bool ConnectionStorage::saveConnection(const ConnectionConfig &config) {
    QSqlQuery query(storageDb);

    query.prepare(R"(
        INSERT OR REPLACE INTO connections
        (name, type, host, port, database_name, username, password, file_path, updated_at)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, CURRENT_TIMESTAMP)
    )");

    query.addBindValue(config.name);
    query.addBindValue(static_cast<int>(config.type));
    query.addBindValue(config.host);
    query.addBindValue(config.port);
    query.addBindValue(config.database);
    query.addBindValue(config.username);
    query.addBindValue(encryptPassword(config.password));
    query.addBindValue(config.filePath);

    if (!query.exec()) {
        qWarning() << "Failed to save connection:" << query.lastError().text();
        return false;
    }

    return true;
}

bool ConnectionStorage::removeConnection(const QString &name) {
    QSqlQuery query(storageDb);
    query.prepare("DELETE FROM connections WHERE name = ?");
    query.addBindValue(name);

    if (!query.exec()) {
        qWarning() << "Failed to remove connection:" << query.lastError().text();
        return false;
    }

    return true;
}

QVector<ConnectionConfig> ConnectionStorage::loadAllConnections() {
    QVector<ConnectionConfig> connections;

    QSqlQuery query(storageDb);
    query.exec("SELECT name, type, host, port, database_name, username, password, file_path FROM connections ORDER BY name");

    while (query.next()) {
        ConnectionConfig config;
        config.name = query.value(0).toString();
        config.type = static_cast<DatabaseType>(query.value(1).toInt());
        config.host = query.value(2).toString();
        config.port = query.value(3).toInt();
        config.database = query.value(4).toString();
        config.username = query.value(5).toString();
        config.password = decryptPassword(query.value(6).toString());
        config.filePath = query.value(7).toString();

        connections.append(config);
    }

    return connections;
}

bool ConnectionStorage::updateConnection(const QString &oldName, const ConnectionConfig &config) {
    // If name changed, we need to delete old and insert new
    if (oldName != config.name) {
        removeConnection(oldName);
    }
    return saveConnection(config);
}
