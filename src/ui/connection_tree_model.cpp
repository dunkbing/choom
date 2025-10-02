#include "ui/connection_tree_model.h"
#include "database/connection_manager.h"
#include <QIcon>
#include <QtConcurrent>
#include <QFutureWatcher>
#include <QPainter>
#include <QPixmap>
#include <QVariant>

namespace {
constexpr int kSpinnerOriginalIconRole = Qt::UserRole + 1024;
}

ConnectionTreeModel::ConnectionTreeModel(QObject *parent)
    : QStandardItemModel(parent) {
    setHorizontalHeaderLabels({"Connections"});

    // Create reusable spinner icon
    spinnerIcon = new SpinnerIcon(this);
    connect(spinnerIcon, &SpinnerIcon::iconUpdated, this, [this](const QIcon &icon) {
        // Update all spinning items with the new icon frame
        for (auto it = spinningItems.begin(); it != spinningItems.end(); ++it) {
            TreeItem *item = it.key();
            item->setIcon(icon);
        }
    });
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

void ConnectionTreeModel::addConnectionPlaceholder(const QString &connectionName, DatabaseType dbType) {
    auto *connectionItem = new TreeItem(connectionName, TreeItemType::Connection);
    connectionItem->setConnectionName(connectionName);
    connectionItem->setIcon(getIconForDatabaseType(dbType));
    connectionItem->setLoaded(false);

    // Add a placeholder child to make it expandable
    auto *placeholder = new TreeItem("Not connected", TreeItemType::Connection);
    connectionItem->appendRow(placeholder);

    appendRow(connectionItem);
}

void ConnectionTreeModel::connectToDatabase(TreeItem *connectionItem) {
    if (!connectionItem || connectionItem->getType() != TreeItemType::Connection) {
        return;
    }

    QString connectionName = connectionItem->getConnectionName();
    emit connectionStarted(connectionName);

    startSpinner(connectionItem);

    // Update placeholder to show connecting with animated spinner
    if (connectionItem->rowCount() > 0) {
        auto *placeholder = dynamic_cast<TreeItem*>(connectionItem->child(0));
        if (placeholder) {
            placeholder->setText("Connecting...");
            startSpinner(placeholder);
        }
    }

    // Connect in background
    auto future = QtConcurrent::run([connectionName]() -> QPair<bool, QString> {
        DatabaseConnection *conn = ConnectionManager::instance().getConnection(connectionName);
        if (conn && conn->connect()) {
            return QPair<bool, QString>(true, QString());
        }
        return QPair<bool, QString>(false, conn ? conn->getLastError() : "Connection not found");
    });

    auto *watcher = new QFutureWatcher<QPair<bool, QString>>(this);
    connect(watcher, &QFutureWatcher<QPair<bool, QString>>::finished, this, [this, watcher, connectionItem, connectionName]() {
        auto result = watcher->result();

        if (result.first) {
            // Stop spinner and remove placeholder
            if (connectionItem->rowCount() > 0) {
                auto *placeholder = dynamic_cast<TreeItem*>(connectionItem->child(0));
                if (placeholder) {
                    stopSpinner(placeholder);
                }
            }
            connectionItem->removeRows(0, connectionItem->rowCount());

            // Load structure
            DatabaseConnection *conn = ConnectionManager::instance().getConnection(connectionName);
            if (conn) {
                connections[connectionName] = conn;
                loadConnectionStructure(connectionItem, conn);
                connectionItem->setLoaded(true);
            }

            stopSpinner(connectionItem);
            emit connectionFinished(connectionName, true, QString());
        } else {
            // Stop spinner and show error in placeholder
            if (connectionItem->rowCount() > 0) {
                auto *placeholder = dynamic_cast<TreeItem*>(connectionItem->child(0));
                if (placeholder) {
                    stopSpinner(placeholder);
                    placeholder->setText("Connection failed");
                }
            }
            stopSpinner(connectionItem);
            emit connectionFinished(connectionName, false, result.second);
        }

        watcher->deleteLater();
    });
    watcher->setFuture(future);
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

void ConnectionTreeModel::loadFolderContentsAsync(TreeItem *folderItem) {
    if (!folderItem || folderItem->isLoaded()) {
        return;
    }

    emit folderLoadingStarted(folderItem);

    // Change placeholder text and start animated spinner
    if (folderItem->rowCount() > 0) {
        auto *placeholder = dynamic_cast<TreeItem*>(folderItem->child(0));
        if (placeholder) {
            placeholder->setText("Loading...");
            startSpinner(placeholder);
        }
    }

    // Get connection info
    QString connectionName = folderItem->getConnectionName();
    QString databaseName = folderItem->getDatabaseName();
    QString schemaName = folderItem->getSchemaName();
    TreeItemType type = folderItem->getType();

    // Load in background
    auto future = QtConcurrent::run([this, connectionName, databaseName, schemaName, type]() -> QStringList {
        DatabaseConnection *connection = connections.value(connectionName);
        if (!connection) {
            return QStringList();
        }

        if (type == TreeItemType::TablesFolder) {
            return connection->getTables(schemaName, databaseName);
        } else if (type == TreeItemType::ViewsFolder) {
            return connection->getViews(schemaName, databaseName);
        } else if (type == TreeItemType::SequencesFolder) {
            return connection->getSequences(schemaName);
        }
        return QStringList();
    });

    auto *watcher = new QFutureWatcher<QStringList>(this);
    connect(watcher, &QFutureWatcher<QStringList>::finished, this, [this, watcher, folderItem, connectionName, databaseName, schemaName, type]() {
        QStringList items = watcher->result();

        // Stop spinner and remove placeholder
        if (folderItem->rowCount() > 0) {
            auto *placeholder = dynamic_cast<TreeItem*>(folderItem->child(0));
            if (placeholder) {
                stopSpinner(placeholder);
            }
        }
        folderItem->removeRows(0, folderItem->rowCount());

        // Add items
        for (const QString &itemName : items) {
            TreeItem *newItem = nullptr;

            if (type == TreeItemType::TablesFolder) {
                newItem = new TreeItem(itemName, TreeItemType::Table);
                newItem->setIcon(getIconForType(TreeItemType::Table));
            } else if (type == TreeItemType::ViewsFolder) {
                newItem = new TreeItem(itemName, TreeItemType::View);
                newItem->setIcon(getIconForType(TreeItemType::View));
            } else if (type == TreeItemType::SequencesFolder) {
                newItem = new TreeItem(itemName, TreeItemType::Sequence);
                newItem->setIcon(getIconForType(TreeItemType::Sequence));
            }

            if (newItem) {
                newItem->setConnectionName(connectionName);
                newItem->setDatabaseName(databaseName);
                newItem->setSchemaName(schemaName);
                folderItem->appendRow(newItem);
            }
        }

        folderItem->setLoaded(true);
        emit folderLoadingFinished(folderItem);
        watcher->deleteLater();
    });
    watcher->setFuture(future);
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

void ConnectionTreeModel::startSpinner(TreeItem *item) {
    if (!item || spinningItems.contains(item)) {
        return;
    }

    if (!item->data(kSpinnerOriginalIconRole).isValid()) {
        item->setData(item->icon(), kSpinnerOriginalIconRole);
    }

    spinningItems[item] = true;
    item->setIcon(spinnerIcon->getIcon());

    if (!spinnerIcon->isRunning()) {
        spinnerIcon->start();
    }
}

void ConnectionTreeModel::stopSpinner(TreeItem *item) {
    if (!item) {
        return;
    }

    spinningItems.remove(item);

    const QVariant originalIcon = item->data(kSpinnerOriginalIconRole);
    if (originalIcon.isValid()) {
        item->setIcon(originalIcon.value<QIcon>());
        item->setData(QVariant(), kSpinnerOriginalIconRole);
    }

    if (spinningItems.isEmpty()) {
        spinnerIcon->stop();
    }
}
