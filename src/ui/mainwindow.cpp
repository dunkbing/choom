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
#include <QTabBar>
#include <QMouseEvent>
#include <QEvent>

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

    // Create tab widget
    tabWidget = new QTabWidget(this);
    tabWidget->setTabsClosable(true);
    tabWidget->setMovable(true);
    tabWidget->setDocumentMode(true);
    tabWidget->tabBar()->setCursor(Qt::PointingHandCursor);
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, [this](int index) {
        QWidget *widget = tabWidget->widget(index);
        tabWidget->removeTab(index);
        widget->deleteLater();
    });

    mainLayout->addWidget(tabWidget);

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

    // Modern dark theme with better contrast
    QString darkStyle = R"(
        QMainWindow, QWidget {
            background-color: #1e1e1e;
            color: #d4d4d4;
        }
        QLineEdit {
            background-color: #2d2d30;
            color: #cccccc;
            border: 1px solid #3e3e42;
            padding: 8px;
            border-radius: 4px;
            selection-background-color: #264f78;
        }
        QLineEdit:focus {
            border: 1px solid #007acc;
        }
        QToolButton {
            background-color: transparent;
            border: none;
            border-radius: 4px;
            padding: 5px;
            text-align: left;
            color: #cccccc;
        }
        QToolButton:hover {
            background-color: #2a2d2e;
        }
        QToolButton:pressed, QToolButton:checked {
            background-color: #094771;
        }
        #sidebarWidget {
            background-color: #252526;
            border: none;
        }
        QTreeView {
            border: none;
            background-color: #252526;
            color: #cccccc;
            outline: none;
        }
        QTreeView::item:hover {
            background-color: #2a2d2e;
        }
        QTreeView::item:selected {
            background-color: #094771;
        }
        QSplitter::handle {
            background-color: #3e3e42;
            width: 1px;
        }
        QSplitter::handle:horizontal {
            width: 2px;
        }
        QSplitter::handle:vertical {
            height: 2px;
        }
        QSplitter::handle:hover {
            background-color: #007acc;
        }
        QTabWidget::pane {
            border: none;
            background-color: #1e1e1e;
            top: -1px;
        }
        QTabBar {
            background-color: #252526;
            border-bottom: 1px solid #3e3e42;
        }
        QTabBar::tab {
            background-color: transparent;
            color: #969696;
            padding: 10px 20px;
            border: none;
            border-bottom: 2px solid transparent;
            margin: 0px;
            min-width: 80px;
        }
        QTabBar::tab:selected {
            background-color: #1e1e1e;
            color: #ffffff;
            border-bottom: 2px solid #007acc;
        }
        QTabBar::close-button {
            image: url(:/icons/assets/close.svg);
            subcontrol-position: right;
            subcontrol-origin: padding;
            padding: 4px;
        }
        QTabBar::close-button:hover {
            background-color: #3e3e42;
            border-radius: 3px;
        }
        QPushButton {
            background-color: #0e639c;
            color: #ffffff;
            border: none;
            padding: 6px 14px;
            border-radius: 2px;
        }
        QPushButton:hover {
            background-color: #1177bb;
        }
        QPushButton:pressed {
            background-color: #0d5689;
        }
        QTableView {
            background-color: #1e1e1e;
            alternate-background-color: #252526;
            color: #cccccc;
            gridline-color: #3e3e42;
            border: none;
        }
        QHeaderView::section {
            background-color: #252526;
            color: #cccccc;
            padding: 5px;
            border: none;
            border-right: 1px solid #3e3e42;
            border-bottom: 1px solid #3e3e42;
        }
        QScrollArea {
            border: none;
            background: transparent;
        }
        QScrollArea > QWidget > QWidget {
            background: transparent;
        }
        QScrollBar:vertical {
            background: #1e1e1e;
            width: 14px;
            border: none;
        }
        QScrollBar::handle:vertical {
            background: #424242;
            border-radius: 7px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background: #4e4e4e;
        }
        QScrollBar:horizontal {
            background: #1e1e1e;
            height: 14px;
            border: none;
        }
        QScrollBar::handle:horizontal {
            background: #424242;
            border-radius: 7px;
            min-width: 20px;
        }
        QScrollBar::handle:horizontal:hover {
            background: #4e4e4e;
        }
        QScrollBar::add-line, QScrollBar::sub-line {
            border: none;
            background: none;
        }
    )";

    setStyleSheet(darkStyle);

    // Initialize welcome widget as first tab
    welcomeWidget = new WelcomeWidget(this);
    tabWidget->addTab(welcomeWidget, "Welcome");

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
    connectionTree->setEditTriggers(QAbstractItemView::NoEditTriggers);

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
        // Load table data in a new tab
        QString connectionName = item->getConnectionName();
        QString tableName = item->text();

        // Get the actual database connection name to use
        DatabaseConnection *conn = ConnectionManager::instance().getConnection(connectionName);
        if (conn && conn->isConnected()) {
            openTableInTab(conn->getConnectionName(), tableName);
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
                openSQLEditor(conn->getConnectionName(), database, schema);
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

void MainWindow::openSQLEditor(const QString &connectionName, const QString &database, const QString &schema) {
    // Create tab name
    QString tabName = "SQL Editor";
    if (!database.isEmpty()) {
        tabName += " - " + database;
    } else if (!schema.isEmpty()) {
        tabName += " - " + schema;
    } else if (!connectionName.isEmpty()) {
        tabName += " - " + connectionName;
    }

    // Check if tab already exists
    int existingTab = findTab(tabName);
    if (existingTab >= 0) {
        tabWidget->setCurrentIndex(existingTab);
        return;
    }

    // Create new SQL editor tab
    auto *sqlEditor = new SQLEditor(this);
    sqlEditor->setDatabaseContext(connectionName, database, schema);
    int tabIndex = tabWidget->addTab(sqlEditor, tabName);
    tabWidget->setCurrentIndex(tabIndex);
}

void MainWindow::openTableInTab(const QString &connectionName, const QString &tableName) {
    QString tabName = tableName;

    // Check if tab already exists
    int existingTab = findTab(tabName);
    if (existingTab >= 0) {
        tabWidget->setCurrentIndex(existingTab);
        return;
    }

    // Create new table viewer tab
    auto *tableViewer = new TableViewer(this);
    tableViewer->loadTableData(connectionName, tableName);
    int tabIndex = tabWidget->addTab(tableViewer, tabName);
    tabWidget->setCurrentIndex(tabIndex);
}

int MainWindow::findTab(const QString &tabName) {
    for (int i = 0; i < tabWidget->count(); ++i) {
        if (tabWidget->tabText(i) == tabName) {
            return i;
        }
    }
    return -1;
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
        return;
    }

    qDebug() << "Loading" << savedConnections.size() << "saved connections";

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
            } else {
                qWarning() << "Failed to connect to saved connection:" << config.name
                          << "-" << conn->getLastError();
            }
        }
    }

}
