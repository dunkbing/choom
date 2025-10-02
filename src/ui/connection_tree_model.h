#ifndef CONNECTION_TREE_MODEL_H
#define CONNECTION_TREE_MODEL_H

#include <QStandardItemModel>
#include <QStandardItem>
#include "../database/database_connection.h"

enum class TreeItemType {
    Connection,
    Database,
    Schema,
    TablesFolder,
    ViewsFolder,
    SequencesFolder,
    Table,
    View,
    Sequence
};

class TreeItem : public QStandardItem {
public:
    TreeItem(const QString &text, TreeItemType type)
        : QStandardItem(text), itemType(type), loaded(false) {}

    TreeItemType getType() const { return itemType; }
    void setConnectionName(const QString &name) { connectionName = name; }
    QString getConnectionName() const { return connectionName; }
    void setDatabaseName(const QString &name) { databaseName = name; }
    QString getDatabaseName() const { return databaseName; }
    void setSchemaName(const QString &name) { schemaName = name; }
    QString getSchemaName() const { return schemaName; }

    bool isLoaded() const { return loaded; }
    void setLoaded(bool value) { loaded = value; }

private:
    TreeItemType itemType;
    QString connectionName;
    QString databaseName;
    QString schemaName;
    bool loaded;
};

class ConnectionTreeModel : public QStandardItemModel {
    Q_OBJECT

public:
    explicit ConnectionTreeModel(QObject *parent = nullptr);

    void addConnection(DatabaseConnection *connection);
    void removeConnection(const QString &connectionName);
    void refreshConnection(const QString &connectionName);
    void loadFolderContents(TreeItem *folderItem);

    TreeItem* findConnectionItem(const QString &connectionName);

private:
    void loadConnectionStructure(TreeItem *connectionItem, DatabaseConnection *connection);
    void loadSQLiteStructure(TreeItem *connectionItem, DatabaseConnection *connection);
    void loadMySQLStructure(TreeItem *connectionItem, DatabaseConnection *connection);
    void loadPostgreSQLStructure(TreeItem *connectionItem, DatabaseConnection *connection);

    QIcon getIconForDatabaseType(DatabaseType type);
    QIcon getIconForType(TreeItemType type);

    // Store connections for lazy loading
    QMap<QString, DatabaseConnection*> connections;
};

#endif // CONNECTION_TREE_MODEL_H
