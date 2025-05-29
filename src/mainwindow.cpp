#include "mainwindow.h"
#include "macos_titlebar.h"
#include "qmlwebview.h"
#include "utils.h"
#include <QApplication>
#include <QLabel>
#include <QMessageBox>
#include <QScrollArea>
#include <QShortcut>
#include <QStylePainter>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWebEngineProfile>
#include <QWebEngineSettings>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUI();
    commandPalette = new CommandPalette(this);
    connect(commandPalette, &CommandPalette::urlSelected, this,
            &MainWindow::handleCommandPaletteUrl);
    addNewTab();

    // Ensure the first tab is displayed properly
    QTimer::singleShot(0, this, [this]() { updateTabButtons(); });

    // web engine settings
    const QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();
    QWebEngineSettings *settings = profile->settings();
    settings->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, true);
    settings->setAttribute(QWebEngineSettings::PlaybackRequiresUserGesture, false);
    settings->setAttribute(QWebEngineSettings::WebGLEnabled, true);
    settings->setAttribute(QWebEngineSettings::Accelerated2dCanvasEnabled, true);
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

    // Create a container widget for the title bar and main content
    auto *containerWidget = new QWidget(this);
    containerWidget->setAttribute(Qt::WA_ContentsMarginsRespectsSafeArea, false);
    auto *containerLayout = new QHBoxLayout(containerWidget);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);

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

    // Add sidebar and content stack to container
    containerLayout->addWidget(sidebarWidget);

    // Add main layout and tab content to container
    auto *contentWidget = new QWidget();
    contentWidget->setAttribute(Qt::WA_ContentsMarginsRespectsSafeArea, false);
    auto *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);
    contentLayout->addWidget(contentStack);

    mainLayout->addWidget(contentWidget);

    containerLayout->addLayout(mainLayout, 1);
    setCentralWidget(containerWidget);

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

    // shortcuts
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_T), this, SLOT(showCommandPalette()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_W), this, [this]() {
        if (webViews.count() > 0) {
            closeTab(currentTabIndex);
        }
    });
}

void MainWindow::setupSidebar() {
    // Create sidebar widget
    sidebarWidget = new QWidget(this);
    sidebarWidget->setObjectName("sidebarWidget");
    sidebarWidget->setFixedWidth(220);

    // Create sidebar layout
    sidebarLayout = new QVBoxLayout(sidebarWidget);
    sidebarWidget->setAttribute(Qt::WA_ContentsMarginsRespectsSafeArea, false);

#ifdef Q_OS_MACOS
    sidebarLayout->setContentsMargins(10, 40, 10, 10);
#else
    sidebarLayout->setContentsMargins(10, 10, 10, 10);
#endif
    sidebarLayout->setSpacing(10);

    // Add URL display
    urlBar = new QLineEdit(sidebarWidget);
    urlBar->setPlaceholderText("Enter URL...");
    urlBar->setObjectName("urlDisplay");
    connect(urlBar, &QLineEdit::returnPressed, this, &MainWindow::navigateToUrl);
    sidebarLayout->addWidget(urlBar);

    // Add spacing before tabs
    sidebarLayout->addSpacing(15);

    // Add "New Tab" button
    auto *newTabButton = new QToolButton(sidebarWidget);
    newTabButton->setText(" New Tab");
    newTabButton->setIcon(Utils::createIconFromResource(":/icons/assets/plus.svg"));
    newTabButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    newTabButton->setProperty("class", "TabButton");
    newTabButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(newTabButton, &QToolButton::clicked, this, [this]() { addNewTab(); });
    sidebarLayout->addWidget(newTabButton);

    // Create QML tabs widget
    tabsWidget = new QmlTabsWidget(sidebarWidget);
    tabsWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    tabsWidget->setMinimumHeight(100);
    connect(tabsWidget, &QmlTabsWidget::tabClicked, this, &MainWindow::tabClicked);
    connect(tabsWidget, &QmlTabsWidget::tabCloseClicked, this, &MainWindow::closeTab);

    sidebarLayout->addWidget(tabsWidget, 1); // Give it stretch factor of 1
}

void MainWindow::updateTabButtons() {
    qDebug() << "updateTabButtons called, webViews count:" << webViews.size();

    tabsWidget->clearTabs();

    for (int i = 0; i < webViews.size(); ++i) {
        QmlWebView *view = webViews[i];
        QString title = view->title();
        if (title.isEmpty()) {
            title = "New Tab";
        }

        qDebug() << "Adding tab" << i << "with title:" << title;

        // Use the favicon URL if available, otherwise QML will use the default
        QString iconUrl = view->faviconUrl();
        if (!iconUrl.isEmpty()) {
            iconUrl = "image://favicon/" + iconUrl; // Reconstruct the image provider URL
        }

        bool isSelected = (i == currentTabIndex);
        tabsWidget->addTab(title, iconUrl, isSelected);
    }

    qDebug() << "updateTabButtons completed";
}

void MainWindow::createWebView(const QUrl &url) {
    auto *webView = new QmlWebView();
    webView->setAttribute(Qt::WA_ContentsMarginsRespectsSafeArea, false);

    // Optimize WebView performance
    webView->setAttribute(Qt::WA_OpaquePaintEvent, true);
    webView->setAttribute(Qt::WA_NoSystemBackground, true);

    // Load the URL
    webView->load(url);
    webViews.append(webView);

    // Create a lightweight container for the webView
    auto *container = new WebViewContainer(webView, this);
    webViewContainers.append(container);

    // Add the container to the content stack
    contentStack->addWidget(container);

    // Set as current tab
    currentTabIndex = webViews.size() - 1;
    contentStack->setCurrentWidget(container);

    connect(webView, &QmlWebView::urlChanged, this, &MainWindow::updateUrlBar);

    // Update tab title when the page title changes
    connect(webView, &QmlWebView::titleChanged, this, [this, webView](const QString &title) {
        int index = webViews.indexOf(webView);
        if (index != -1) {
            // Update tab buttons
            updateTabButtons();
        }
    });

    // update favicon when it changes
    connect(webView, &QmlWebView::iconChanged, this, [this, webView](const QIcon &icon) {
        // Force update of tab buttons when icon changes
        updateTabButtons();
    });

    // Update tabs immediately if tabsWidget is ready
    if (tabsWidget) {
        updateTabButtons();
    }
    webView->setFocus();
}

QmlWebView *MainWindow::currentWebView() const {
    if (webViews.isEmpty() || currentTabIndex < 0 || currentTabIndex >= webViews.size()) {
        return nullptr;
    }
    return webViews[currentTabIndex];
}

void MainWindow::navigateToUrl() const {
    QmlWebView *webView = currentWebView();
    if (!webView) {
        return;
    }

    QString url = Utils::normalizeUrl(urlBar->text());
    webView->load(QUrl(url));
}

void MainWindow::updateUrlBar(const QUrl &url) const {
    // Only update if the sender is the current tab
    if (auto *webView = qobject_cast<QmlWebView *>(sender()); webView == currentWebView()) {
        urlBar->setText(Utils::createDisplayUrl(url));
    }
}

void MainWindow::goBack() const {
    QmlWebView *webView = currentWebView();
    if (webView) {
        webView->back();
    }
}

void MainWindow::goForward() const {
    QmlWebView *webView = currentWebView();
    if (webView) {
        webView->forward();
    }
}

void MainWindow::reload() const {
    QmlWebView *webView = currentWebView();
    if (webView) {
        webView->reload();
    }
}

void MainWindow::showCommandPalette() const {
    if (commandPalette) {
        commandPalette->exec();
    }
}

void MainWindow::handleCommandPaletteUrl(const QUrl &url) {
    addNewTab(url);
}

void MainWindow::addNewTab(const QUrl &url) {
    createWebView(url);
}

void MainWindow::closeTab(const int index) {
    // keep at least one tab open
    if (webViews.count() <= 1) {
        QMessageBox::information(this, "Cannot Close Tab",
                                 "Cannot close the last tab. Application would exit instead.");
        return;
    }

    // Remove from both lists
    WebViewContainer *container = webViewContainers.takeAt(index);
    QmlWebView *webView = webViews.takeAt(index);

    contentStack->removeWidget(container);

    // Delete the container which owns the webView
    delete container;

    // update current index
    if (currentTabIndex >= webViews.size()) {
        currentTabIndex = webViews.size() - 1;
    }

    // show the new current tab
    if (currentTabIndex >= 0 && currentTabIndex < webViews.size()) {
        contentStack->setCurrentWidget(webViewContainers[currentTabIndex]);

        const QmlWebView *view = webViews[currentTabIndex];
        urlBar->setText(Utils::createDisplayUrl(view->url()));
    }

    updateTabButtons();
}

void MainWindow::tabClicked(const int index) {
    if (index != currentTabIndex && index >= 0 && index < webViews.size()) {
        currentTabIndex = index;
        contentStack->setCurrentWidget(webViewContainers[index]);

        // Update URL bar
        QmlWebView *view = webViews[index];
        urlBar->setText(Utils::createDisplayUrl(view->url()));

        updateTabButtons();
    }
}
