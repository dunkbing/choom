#ifndef CONNECTION_TREE_MODEL_H
#define CONNECTION_TREE_MODEL_H

#include <QStandardItemModel>
#include <QStandardItem>
#include <QTimer>
#include "../database/database_connection.h"
#include "spinner_icon.h"

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
    void addConnectionPlaceholder(const QString &connectionName, DatabaseType dbType);
    void removeConnection(const QString &connectionName);
    void refreshConnection(const QString &connectionName);
    void loadFolderContents(TreeItem *folderItem);
    void loadFolderContentsAsync(TreeItem *folderItem);
    void connectToDatabase(TreeItem *connectionItem);

    TreeItem* findConnectionItem(const QString &connectionName);

signals:
    void folderLoadingStarted(TreeItem *folderItem);
    void folderLoadingFinished(TreeItem *folderItem);
    void connectionStarted(const QString &connectionName);
    void connectionFinished(const QString &connectionName, bool success, const QString &error);

private:
    void loadConnectionStructure(TreeItem *connectionItem, DatabaseConnection *connection);
    void loadSQLiteStructure(TreeItem *connectionItem, DatabaseConnection *connection);
    void loadMySQLStructure(TreeItem *connectionItem, DatabaseConnection *connection);
    void loadPostgreSQLStructure(TreeItem *connectionItem, DatabaseConnection *connection);

    QIcon getIconForDatabaseType(DatabaseType type);
    QIcon getIconForType(TreeItemType type);
    void startSpinner(TreeItem *item);
    void stopSpinner(TreeItem *item);

    // Store connections for lazy loading
    QMap<QString, DatabaseConnection*> connections;

    // Spinner
    SpinnerIcon *spinnerIcon;
    QMap<TreeItem*, bool> spinningItems;  // items currently showing spinner
};

#endif // CONNECTION_TREE_MODEL_H
