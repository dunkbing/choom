#include "connection_tree_model.h"
#include <QIcon>

ConnectionTreeModel::ConnectionTreeModel(QObject *parent)
    : QStandardItemModel(parent) {
    setHorizontalHeaderLabels({"Connections"});
}

void ConnectionTreeModel::addConnection(DatabaseConnection *connection) {
    if (!connection) return;

    // Store connection reference for lazy loading
    connections[connection->getName()] = connection;

    auto *connectionItem = new TreeItem(connection->getName(), TreeItemType::Connection);
    connectionItem->setConnectionName(connection->getName());
    connectionItem->setIcon(getIconForDatabaseType(connection->getType()));

    appendRow(connectionItem);

    // Load structure (now with lazy loading for folders)
    loadConnectionStructure(connectionItem, connection);
}

void ConnectionTreeModel::removeConnection(const QString &connectionName) {
    TreeItem *item = findConnectionItem(connectionName);
    if (item) {
        removeRow(item->row());
    }
    // Remove from connections map
    connections.remove(connectionName);
}

void ConnectionTreeModel::refreshConnection(const QString &connectionName) {
    TreeItem *item = findConnectionItem(connectionName);
    if (item) {
        item->removeRows(0, item->rowCount());
        // Re-load structure
        // Would need to get connection from manager
    }
}

TreeItem* ConnectionTreeModel::findConnectionItem(const QString &connectionName) {
    for (int i = 0; i < rowCount(); ++i) {
        auto *item = dynamic_cast<TreeItem*>(this->item(i));
        if (item && item->getConnectionName() == connectionName) {
            return item;
        }
    }
    return nullptr;
}

void ConnectionTreeModel::loadConnectionStructure(TreeItem *connectionItem, DatabaseConnection *connection) {
    switch (connection->getType()) {
        case DatabaseType::SQLite:
            loadSQLiteStructure(connectionItem, connection);
            break;
        case DatabaseType::MySQL:
            loadMySQLStructure(connectionItem, connection);
            break;
        case DatabaseType::PostgreSQL:
            loadPostgreSQLStructure(connectionItem, connection);
            break;
    }
}

void ConnectionTreeModel::loadSQLiteStructure(TreeItem *connectionItem, DatabaseConnection *connection) {
    // Tables folder - create empty with placeholder
    auto *tablesFolder = new TreeItem("Tables", TreeItemType::TablesFolder);
    tablesFolder->setIcon(getIconForType(TreeItemType::TablesFolder));
    tablesFolder->setConnectionName(connection->getName());
    tablesFolder->setLoaded(false);
    connectionItem->appendRow(tablesFolder);

    // Add placeholder to make it expandable
    auto *placeholder = new TreeItem("Loading...", TreeItemType::TablesFolder);
    tablesFolder->appendRow(placeholder);

    // Views folder - create empty with placeholder
    auto *viewsFolder = new TreeItem("Views", TreeItemType::ViewsFolder);
    viewsFolder->setIcon(getIconForType(TreeItemType::ViewsFolder));
    viewsFolder->setConnectionName(connection->getName());
    viewsFolder->setLoaded(false);
    connectionItem->appendRow(viewsFolder);

    // Add placeholder to make it expandable
    auto *viewPlaceholder = new TreeItem("Loading...", TreeItemType::ViewsFolder);
    viewsFolder->appendRow(viewPlaceholder);
}

void ConnectionTreeModel::loadMySQLStructure(TreeItem *connectionItem, DatabaseConnection *connection) {
    // Databases folder
    QStringList databases = connection->getDatabases();

    for (const QString &database : databases) {
        auto *dbItem = new TreeItem(database, TreeItemType::Database);
        dbItem->setConnectionName(connection->getName());
        dbItem->setDatabaseName(database);
        dbItem->setIcon(getIconForType(TreeItemType::Database));
        connectionItem->appendRow(dbItem);

        // Tables folder - create empty with placeholder
        auto *tablesFolder = new TreeItem("Tables", TreeItemType::TablesFolder);
        tablesFolder->setIcon(getIconForType(TreeItemType::TablesFolder));
        tablesFolder->setConnectionName(connection->getName());
        tablesFolder->setDatabaseName(database);
        tablesFolder->setLoaded(false);
        dbItem->appendRow(tablesFolder);

        // Add placeholder to make it expandable
        auto *placeholder = new TreeItem("Loading...", TreeItemType::TablesFolder);
        tablesFolder->appendRow(placeholder);

        // Views folder - create empty with placeholder
        auto *viewsFolder = new TreeItem("Views", TreeItemType::ViewsFolder);
        viewsFolder->setIcon(getIconForType(TreeItemType::ViewsFolder));
        viewsFolder->setConnectionName(connection->getName());
        viewsFolder->setDatabaseName(database);
        viewsFolder->setLoaded(false);
        dbItem->appendRow(viewsFolder);

        // Add placeholder to make it expandable
        auto *viewPlaceholder = new TreeItem("Loading...", TreeItemType::ViewsFolder);
        viewsFolder->appendRow(viewPlaceholder);
    }
}

void ConnectionTreeModel::loadPostgreSQLStructure(TreeItem *connectionItem, DatabaseConnection *connection) {
    // Schemas
    QStringList schemas = connection->getSchemas();

    for (const QString &schema : schemas) {
        auto *schemaItem = new TreeItem(schema, TreeItemType::Schema);
        schemaItem->setConnectionName(connection->getName());
        schemaItem->setSchemaName(schema);
        schemaItem->setIcon(getIconForType(TreeItemType::Schema));
        connectionItem->appendRow(schemaItem);

        // Tables folder - create empty with placeholder
        auto *tablesFolder = new TreeItem("Tables", TreeItemType::TablesFolder);
        tablesFolder->setIcon(getIconForType(TreeItemType::TablesFolder));
        tablesFolder->setConnectionName(connection->getName());
        tablesFolder->setSchemaName(schema);
        tablesFolder->setLoaded(false);
        schemaItem->appendRow(tablesFolder);

        // Add placeholder to make it expandable
        auto *placeholder = new TreeItem("Loading...", TreeItemType::TablesFolder);
        tablesFolder->appendRow(placeholder);

        // Views folder - create empty with placeholder
        auto *viewsFolder = new TreeItem("Views", TreeItemType::ViewsFolder);
        viewsFolder->setIcon(getIconForType(TreeItemType::ViewsFolder));
        viewsFolder->setConnectionName(connection->getName());
        viewsFolder->setSchemaName(schema);
        viewsFolder->setLoaded(false);
        schemaItem->appendRow(viewsFolder);

        // Add placeholder to make it expandable
        auto *viewPlaceholder = new TreeItem("Loading...", TreeItemType::ViewsFolder);
        viewsFolder->appendRow(viewPlaceholder);

        // Sequences folder - create empty with placeholder
        auto *sequencesFolder = new TreeItem("Sequences", TreeItemType::SequencesFolder);
        sequencesFolder->setIcon(getIconForType(TreeItemType::SequencesFolder));
        sequencesFolder->setConnectionName(connection->getName());
        sequencesFolder->setSchemaName(schema);
        sequencesFolder->setLoaded(false);
        schemaItem->appendRow(sequencesFolder);

        // Add placeholder to make it expandable
        auto *seqPlaceholder = new TreeItem("Loading...", TreeItemType::SequencesFolder);
        sequencesFolder->appendRow(seqPlaceholder);
    }
}

void ConnectionTreeModel::loadFolderContents(TreeItem *folderItem) {
    if (!folderItem || folderItem->isLoaded()) {
        return;
    }

    // Remove placeholder
    folderItem->removeRows(0, folderItem->rowCount());

    // Get connection
    QString connectionName = folderItem->getConnectionName();
    DatabaseConnection *connection = connections.value(connectionName);
    if (!connection) {
        return;
    }

    TreeItemType type = folderItem->getType();
    QString databaseName = folderItem->getDatabaseName();
    QString schemaName = folderItem->getSchemaName();

    if (type == TreeItemType::TablesFolder) {
        // Load tables
        QStringList tables = connection->getTables(schemaName, databaseName);
        for (const QString &table : tables) {
            auto *tableItem = new TreeItem(table, TreeItemType::Table);
            tableItem->setConnectionName(connectionName);
            tableItem->setDatabaseName(databaseName);
            tableItem->setSchemaName(schemaName);
            tableItem->setIcon(getIconForType(TreeItemType::Table));
            folderItem->appendRow(tableItem);
        }
    } else if (type == TreeItemType::ViewsFolder) {
        // Load views
        QStringList views = connection->getViews(schemaName, databaseName);
        for (const QString &view : views) {
            auto *viewItem = new TreeItem(view, TreeItemType::View);
            viewItem->setConnectionName(connectionName);
            viewItem->setDatabaseName(databaseName);
            viewItem->setSchemaName(schemaName);
            viewItem->setIcon(getIconForType(TreeItemType::View));
            folderItem->appendRow(viewItem);
        }
    } else if (type == TreeItemType::SequencesFolder) {
        // Load sequences (PostgreSQL only)
        QStringList sequences = connection->getSequences(schemaName);
        for (const QString &sequence : sequences) {
            auto *seqItem = new TreeItem(sequence, TreeItemType::Sequence);
            seqItem->setConnectionName(connectionName);
            seqItem->setSchemaName(schemaName);
            seqItem->setIcon(getIconForType(TreeItemType::Sequence));
            folderItem->appendRow(seqItem);
        }
    }

    folderItem->setLoaded(true);
}

QIcon ConnectionTreeModel::getIconForDatabaseType(DatabaseType type) {
    switch (type) {
        case DatabaseType::SQLite:
            return QIcon(":/icons/assets/icons/sqlite.svg");
        case DatabaseType::MySQL:
            return QIcon(":/icons/assets/icons/mysql.svg");
        case DatabaseType::PostgreSQL:
            return QIcon(":/icons/assets/icons/postgres.svg");
        default:
            return QIcon(":/icons/assets/icons/connection.svg");
    }
}

QIcon ConnectionTreeModel::getIconForType(TreeItemType type) {
    switch (type) {
        case TreeItemType::Connection:
            return QIcon(":/icons/assets/icons/connection.svg");
        case TreeItemType::Database:
            return QIcon(":/icons/assets/icons/database.svg");
        case TreeItemType::Schema:
            return QIcon(":/icons/assets/icons/schema.svg");
        case TreeItemType::TablesFolder:
        case TreeItemType::ViewsFolder:
        case TreeItemType::SequencesFolder:
            return QIcon(":/icons/assets/icons/folder.svg");
        case TreeItemType::Table:
            return QIcon(":/icons/assets/icons/table.svg");
        case TreeItemType::View:
            return QIcon(":/icons/assets/icons/view.svg");
        case TreeItemType::Sequence:
            return QIcon(":/icons/assets/icons/sequence.svg");
        default:
            return QIcon();
    }
}
