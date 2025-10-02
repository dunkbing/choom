#include "mainwindow.h"
#include "../macos_titlebar.h"
#include "../core/utils.h"
#include "../core/connection_storage.h"
#include <QApplication>
#include <QLabel>
#include <QMessageBox>
#include <QScrollArea>
#include <QShortcut>
#include <QHBoxLayout>
#include <QToolButton>
#include <QSqlQuery>
#include <QMenu>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    // Initialize connection storage
    if (!ConnectionStorage::instance().initialize()) {
        QMessageBox::warning(this, "Storage Error",
            "Failed to initialize connection storage. Connections will not be saved.");
    }

    setupUI();
    loadSavedConnections();
}

void MainWindow::setupUI() {
#ifdef Q_OS_MACOS
    titleBar = nullptr; // no custom title bar on macOS
    MacOSTitleBar::setupToolbar(this);
#else
    // Create title bar
    titleBar = new TitleBar(this);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    connect(titleBar, &TitleBar::minimizeClicked, this, &MainWindow::showMinimized);
    connect(titleBar, &TitleBar::maximizeClicked, this, [this]() {
        if (isMaximized()) {
            showNormal();
        } else {
            showMaximized();
        }
    });
    connect(titleBar, &TitleBar::closeClicked, this, &MainWindow::close);
#endif
    setAttribute(Qt::WA_ContentsMarginsRespectsSafeArea, false);

    setupSidebar();

    // Create main content layout
    auto *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

#ifndef Q_OS_MACOS
    // Add custom title bar for non-macOS platforms
    mainLayout->addWidget(titleBar);
#endif

    // Create the content stacked widget
    contentStack = new QStackedWidget(this);
    contentStack->setObjectName("contentStack");
    contentStack->setContentsMargins(0, 0, 0, 0);

    // Create content widget
    auto *contentWidget = new QWidget();
    contentWidget->setAttribute(Qt::WA_ContentsMarginsRespectsSafeArea, false);
    auto *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);
    contentLayout->addWidget(contentStack);

    mainLayout->addWidget(contentWidget);

    // Create splitter for resizable sidebar
    splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(sidebarWidget);

    auto *mainContentWidget = new QWidget();
    mainContentWidget->setLayout(mainLayout);
    splitter->addWidget(mainContentWidget);

    // Set initial sizes: sidebar 250px, rest of space for content
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({250, 750});

    setCentralWidget(splitter);

    // window props
    resize(1024, 768);

    // dark theme
    QString darkStyle = R"(
        QMainWindow, QWidget {
            background-color: #24262e;
            color: #ffffff;
        }
        QLineEdit {
            background-color: #36383e;
            color: #ffffff;
            border: 1px solid #444;
            padding: 8px;
            border-radius: 4px;
            selection-background-color: #4d78cc;
        }
        QToolButton {
            background-color: transparent;
            border: none;
            border-radius: 4px;
            padding: 5px;
            text-align: left;
        }
        QToolButton:hover {
            background-color: #454750;
        }
        QToolButton:pressed, QToolButton:checked {
            background-color: #303238;
        }
        #sidebarWidget {
            background-color: #24262e;
            border: none;
        }
        QTreeView {
            border: none;
            background-color: #24262e;
        }
        QSplitter::handle {
            background-color: #444;
            width: 1px;
        }
        QSplitter::handle:horizontal {
            width: 2px;
        }
        QSplitter::handle:vertical {
            height: 2px;
        }
        QSplitter::handle:hover {
            background-color: #666;
        }
        #urlDisplay {
            background-color: #36383e;
            color: #ffffff;
            border: 1px solid #444;
            border-radius: 4px;
            padding: 8px;
            font-size: 12px;
        }
        .TabButton {
            text-align: left;
            padding-left: 10px;
            border-radius: 4px;
            min-height: 25px;
            max-height: 25px;
        }
        .TabButton[selected="true"] {
            background-color: #454750;
        }
        QScrollArea {
            border: none;
            background: transparent;
        }
        QScrollArea > QWidget > QWidget {
            background: transparent;
        }
    )";

    setStyleSheet(darkStyle);

    // Initialize content widgets
    welcomeWidget = new WelcomeWidget(this);
    tableViewer = new TableViewer(this);
    sqlEditor = new SQLEditor(this);

    contentStack->addWidget(welcomeWidget);
    contentStack->addWidget(tableViewer);
    contentStack->addWidget(sqlEditor);

    // Show welcome screen by default
    contentStack->setCurrentWidget(welcomeWidget);

    // shortcuts
    auto *newConnShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_T), this);
    connect(newConnShortcut, &QShortcut::activated, this, &MainWindow::addNewConnection);
}

void MainWindow::setupSidebar() {
    // Create sidebar widget
    sidebarWidget = new QWidget(this);
    sidebarWidget->setObjectName("sidebarWidget");
    sidebarWidget->setMinimumWidth(200);
    sidebarWidget->setMaximumWidth(600);

    // Create sidebar layout
    sidebarLayout = new QVBoxLayout(sidebarWidget);
    sidebarWidget->setAttribute(Qt::WA_ContentsMarginsRespectsSafeArea, false);

#ifdef Q_OS_MACOS
    sidebarLayout->setContentsMargins(10, 40, 10, 10);
#else
    sidebarLayout->setContentsMargins(10, 10, 10, 10);
#endif
    sidebarLayout->setSpacing(10);

    // Add "New Connection" button
    auto *newConnectionButton = new QToolButton(sidebarWidget);
    newConnectionButton->setText(" New Connection");
    newConnectionButton->setIcon(Utils::createIconFromResource(":/icons/assets/plus.svg"));
    newConnectionButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    newConnectionButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(newConnectionButton, &QToolButton::clicked, this, &MainWindow::addNewConnection);
    sidebarLayout->addWidget(newConnectionButton);

    // Add spacing
    sidebarLayout->addSpacing(10);

    // Create connection tree
    connectionTree = new QTreeView(sidebarWidget);
    treeModel = new ConnectionTreeModel(this);
    connectionTree->setModel(treeModel);
    connectionTree->setHeaderHidden(true);
    connectionTree->setAnimated(true);
    connectionTree->setExpandsOnDoubleClick(false);
    connectionTree->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(connectionTree, &QTreeView::doubleClicked, this, &MainWindow::onTreeItemDoubleClicked);
    connect(connectionTree, &QTreeView::customContextMenuRequested, this, &MainWindow::onTreeItemContextMenu);
    connect(connectionTree, &QTreeView::expanded, this, [this](const QModelIndex &index) {
        auto *item = dynamic_cast<TreeItem*>(treeModel->itemFromIndex(index));
        if (item && !item->isLoaded() &&
            (item->getType() == TreeItemType::TablesFolder ||
             item->getType() == TreeItemType::ViewsFolder ||
             item->getType() == TreeItemType::SequencesFolder)) {
            treeModel->loadFolderContents(item);
        }
    });

    sidebarLayout->addWidget(connectionTree, 1);
}

void MainWindow::addNewConnection() {
    ConnectionDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        ConnectionConfig config = dialog.getConnectionConfig();

        // Add connection to manager
        ConnectionManager::instance().addConnection(config);

        // Get the connection and try to connect
        DatabaseConnection *conn = ConnectionManager::instance().getConnection(config.name);
        if (conn) {
            if (conn->connect()) {
                // Save to storage
                if (ConnectionStorage::instance().saveConnection(config)) {
                    qDebug() << "Connection saved to storage:" << config.name;
                } else {
                    qWarning() << "Failed to save connection to storage:" << config.name;
                }

                // Add to tree
                treeModel->addConnection(conn);

                // Set SQL editor context
                sqlEditor->setDatabaseContext(conn->getConnectionName());

                QMessageBox::information(this, "Success",
                    QString("Connected to '%1' successfully!").arg(config.name));
            } else {
                QMessageBox::critical(this, "Connection Failed",
                    QString("Failed to connect to '%1':\n%2")
                        .arg(config.name)
                        .arg(conn->getLastError()));
            }
        }
    }
}

void MainWindow::onTreeItemDoubleClicked(const QModelIndex &index) {
    if (!index.isValid()) return;

    auto *item = dynamic_cast<TreeItem*>(treeModel->itemFromIndex(index));
    if (!item) return;

    if (item->getType() == TreeItemType::Table) {
        // Load table data
        QString connectionName = item->getConnectionName();
        QString tableName = item->text();

        // Get the actual database connection name to use
        DatabaseConnection *conn = ConnectionManager::instance().getConnection(connectionName);
        if (conn && conn->isConnected()) {
            tableViewer->loadTableData(conn->getConnectionName(), tableName);
            contentStack->setCurrentWidget(tableViewer);
        }
    }
}

void MainWindow::onTreeItemContextMenu(const QPoint &pos) {
    QModelIndex index = connectionTree->indexAt(pos);
    if (!index.isValid()) return;

    auto *item = dynamic_cast<TreeItem*>(treeModel->itemFromIndex(index));
    if (!item) return;

    QMenu contextMenu(this);

    // Add "Open SQL Editor" for connection, database, or schema items
    if (item->getType() == TreeItemType::Connection ||
        item->getType() == TreeItemType::Database ||
        item->getType() == TreeItemType::Schema) {

        QAction *sqlEditorAction = contextMenu.addAction("Open SQL Editor");
        connect(sqlEditorAction, &QAction::triggered, this, [this, item]() {
            // Get connection
            DatabaseConnection *conn = ConnectionManager::instance().getConnection(item->getConnectionName());
            if (conn && conn->isConnected()) {
                // Set context based on item type
                QString database = item->getDatabaseName();
                QString schema = item->getSchemaName();
                sqlEditor->setDatabaseContext(conn->getConnectionName(), database, schema);
                contentStack->setCurrentWidget(sqlEditor);
            }
        });
    }

    // Add "Refresh" for all items
    contextMenu.addSeparator();
    QAction *refreshAction = contextMenu.addAction("Refresh");
    connect(refreshAction, &QAction::triggered, this, [this, item]() {
        treeModel->refreshConnection(item->getConnectionName());
    });

    // Add "Delete Connection" for connection items
    if (item->getType() == TreeItemType::Connection) {
        contextMenu.addSeparator();
        QAction *deleteAction = contextMenu.addAction("Delete Connection");
        connect(deleteAction, &QAction::triggered, this, [this, item]() {
            QString connectionName = item->getConnectionName();

            QMessageBox::StandardButton reply = QMessageBox::question(
                this,
                "Delete Connection",
                QString("Are you sure you want to delete the connection '%1'?").arg(connectionName),
                QMessageBox::Yes | QMessageBox::No
            );

            if (reply == QMessageBox::Yes) {
                // Remove from storage
                ConnectionStorage::instance().removeConnection(connectionName);

                // Remove from manager
                ConnectionManager::instance().removeConnection(connectionName);

                // Remove from tree
                treeModel->removeConnection(connectionName);

                qDebug() << "Deleted connection:" << connectionName;
            }
        });
    }

    // Show the context menu at the cursor position
    contextMenu.exec(connectionTree->viewport()->mapToGlobal(pos));
}

void MainWindow::openSQLEditor() {
    contentStack->setCurrentWidget(sqlEditor);
}

void MainWindow::toggleSidebar() {
    if (sidebarWidget->isVisible()) {
        sidebarWidget->hide();
    } else {
        sidebarWidget->show();
    }
}

void MainWindow::loadSavedConnections() {
    // Load all saved connections from storage
    QVector<ConnectionConfig> savedConnections = ConnectionStorage::instance().loadAllConnections();

    if (savedConnections.isEmpty()) {
        qDebug() << "No saved connections found";
        contentStack->setCurrentWidget(welcomeWidget);
        return;
    }

    qDebug() << "Loading" << savedConnections.size() << "saved connections";

    bool firstConnectionSet = false;
    for (const ConnectionConfig &config : savedConnections) {
        // Add connection to manager
        ConnectionManager::instance().addConnection(config);

        // Get the connection and try to connect
        DatabaseConnection *conn = ConnectionManager::instance().getConnection(config.name);
        if (conn) {
            if (conn->connect()) {
                qDebug() << "Successfully connected to:" << config.name;
                // Add to tree
                treeModel->addConnection(conn);

                // Set SQL editor context for the first successfully connected connection
                if (!firstConnectionSet) {
                    sqlEditor->setDatabaseContext(conn->getConnectionName());
                    firstConnectionSet = true;
                }
            } else {
                qWarning() << "Failed to connect to saved connection:" << config.name
                          << "-" << conn->getLastError();
            }
        }
    }

    // Show welcome screen if no connections loaded successfully
    if (treeModel->rowCount() == 0) {
        contentStack->setCurrentWidget(welcomeWidget);
    }
}
