#include "mainwindow.h"
#include "macos_titlebar.h"
#include "utils.h"
#include <QApplication>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QScrollArea>
#include <QShortcut>
#include <QStylePainter>
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
    setAttribute(Qt::WA_TranslucentBackground);
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
    MacOSTitleBar::hideWindowTitleBar(this);

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
            border-right: 1px solid #444;
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

#ifdef Q_OS_MACOS
    sidebarLayout->setContentsMargins(10, 30, 10, 10);
#else
    sidebarLayout->setContentsMargins(10, 10, 10, 10);
#endif
    sidebarLayout->setSpacing(10);

    // Add URL display (simplified)
    urlBar = new QLineEdit(sidebarWidget);
    urlBar->setPlaceholderText("Enter URL...");
    urlBar->setObjectName("urlDisplay");
    connect(urlBar, &QLineEdit::returnPressed, this, &MainWindow::navigateToUrl);
    sidebarLayout->addWidget(urlBar);

    // Add spacing before tabs
    sidebarLayout->addSpacing(15);

    // Add "New Tab" button that looks like tab buttons
    auto *newTabButton = new QToolButton(sidebarWidget);
    newTabButton->setText(" New Tab");
    newTabButton->setIcon(Utils::createIconFromResource(":/icons/assets/plus.svg"));
    newTabButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    newTabButton->setProperty("class", "TabButton");
    newTabButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(newTabButton, &QToolButton::clicked, this, [this]() { addNewTab(); });
    sidebarLayout->addWidget(newTabButton);

    // Create scrollable tabs container
    auto *scrollArea = new QScrollArea(sidebarWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    tabsContainer = new QWidget(scrollArea);
    tabsLayout = new QVBoxLayout(tabsContainer);
    tabsLayout->setContentsMargins(0, 2, 0, 2);
    tabsLayout->setSpacing(2);
    tabsLayout->setAlignment(Qt::AlignTop);

    scrollArea->setWidget(tabsContainer);
    sidebarLayout->addWidget(scrollArea);
}

QToolButton *MainWindow::createTabButton(const QString &title, int index) {
    auto *button = new QToolButton(tabsContainer);
    button->setText(" " + title);
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    button->setCheckable(true);
    button->setProperty("class", "TabButton");
    button->setProperty("index", index);
    // full width
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QWebEngineView *view = webViews[index];
    if (!view->icon().isNull()) {
        button->setIcon(view->icon());
    }

    auto *closeButton = new QToolButton(button);
    closeButton->setIcon(
        Utils::createIconFromResource(":/icons/assets/close.svg", QColor(200, 200, 200)));
    closeButton->setStyleSheet("QToolButton { background: transparent; border: none; }");

    auto *layout = new QHBoxLayout(button);
    layout->setContentsMargins(5, 0, 5, 0);
    layout->setSpacing(5);
    layout->addStretch();
    layout->addWidget(closeButton);

    connect(button, &QToolButton::clicked, this, [this, index]() { tabClicked(index); });
    connect(closeButton, &QToolButton::clicked, this, [this, index]() { closeTab(index); });

    return button;
}

void MainWindow::updateTabButtons() {
    while (QLayoutItem *item = tabsLayout->takeAt(0)) {
        if (QWidget *widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }

    for (int i = 0; i < webViews.size(); ++i) {
        QWebEngineView *view = webViews[i];
        QString title = view->title();
        if (title.isEmpty()) {
            title = "New Tab";
        } else {
            title = Utils::truncateString(title, 20);
        }

        QToolButton *button = createTabButton(title, i);
        button->setChecked(i == currentTabIndex);
        button->setProperty("selected", i == currentTabIndex);

        tabsLayout->addWidget(button);
    }
}

void MainWindow::createWebView(const QUrl &url) {
    auto *webView = new QWebEngineView();
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

    connect(webView, &QWebEngineView::urlChanged, this, &MainWindow::updateUrlBar);

    // Update tab title when the page title changes
    connect(webView, &QWebEngineView::titleChanged, this, [this, webView](const QString &title) {
        int index = webViews.indexOf(webView);
        if (index != -1) {
            // Update tab buttons
            updateTabButtons();
        }
    });

    // update favicon when it changes
    connect(webView, &QWebEngineView::iconChanged, this, [this, webView](const QIcon &icon) {
        // Force update of tab buttons when icon changes
        updateTabButtons();
    });

    updateTabButtons();
    webView->setFocus();
}

QWebEngineView *MainWindow::currentWebView() const {
    if (webViews.isEmpty() || currentTabIndex < 0 || currentTabIndex >= webViews.size()) {
        return nullptr;
    }
    return webViews[currentTabIndex];
}

void MainWindow::navigateToUrl() const {
    QWebEngineView *webView = currentWebView();
    if (!webView) {
        return;
    }

    QString url = Utils::normalizeUrl(urlBar->text());
    webView->load(QUrl(url));
}

void MainWindow::updateUrlBar(const QUrl &url) const {
    // Only update if the sender is the current tab
    if (auto *webView = qobject_cast<QWebEngineView *>(sender()); webView == currentWebView()) {
        urlBar->setText(Utils::createDisplayUrl(url));
    }
}

void MainWindow::goBack() const {
    QWebEngineView *webView = currentWebView();
    if (webView) {
        webView->back();
    }
}

void MainWindow::goForward() const {
    QWebEngineView *webView = currentWebView();
    if (webView) {
        webView->forward();
    }
}

void MainWindow::reload() const {
    QWebEngineView *webView = currentWebView();
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

    WebViewContainer *container = webViewContainers.takeAt(index);
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

        const QWebEngineView *view = webViews[currentTabIndex];
        urlBar->setText(Utils::createDisplayUrl(view->url()));
    }

    updateTabButtons();
}

void MainWindow::tabClicked(const int index) {
    if (index != currentTabIndex && index >= 0 && index < webViews.size()) {
        currentTabIndex = index;
        contentStack->setCurrentWidget(webViewContainers[index]);

        // Update URL bar
        QWebEngineView *view = webViews[index];
        urlBar->setText(Utils::createDisplayUrl(view->url()));

        updateTabButtons();
    }
}
