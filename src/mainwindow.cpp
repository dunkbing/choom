#include "mainwindow.h"
#include "utils.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QApplication>
#include <QMessageBox>
#include <QShortcut>
#include <QStylePainter>
#include <QMouseEvent>
#include <QToolButton>
#include <QStyle>
#include <QScrollArea>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), isDragging(false)
{
    setupUI();
    addNewTab();
}

void MainWindow::setupUI()
{
    // Create title bar
    titleBar = new TitleBar(this);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    setAttribute(Qt::WA_TranslucentBackground);
    connect(titleBar, &TitleBar::minimizeClicked, this, &MainWindow::showMinimized);
    connect(titleBar, &TitleBar::maximizeClicked, this, [this]() {
        if (isMaximized()) {
            showNormal();
        } else {
            showMaximized();
        }
    });
    connect(titleBar, &TitleBar::closeClicked, this, &MainWindow::close);

    // Create a container widget for the title bar and main content
    auto* containerWidget = new QWidget(this);
    auto* containerLayout = new QHBoxLayout(containerWidget);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);

    // Create main content layout
    auto* mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(titleBar);

    // Create the sidebar
    setupSidebar();

    // Create the content stacked widget
    contentStack = new QStackedWidget(this);
    contentStack->setObjectName("contentStack");

    // Add sidebar and content stack to container
    containerLayout->addWidget(sidebarWidget);

    // Add main layout and tab content to container
    auto* contentWidget = new QWidget();
    auto* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);
    contentLayout->addWidget(contentStack);

    mainLayout->addWidget(contentWidget);

    containerLayout->addLayout(mainLayout, 1);
    setCentralWidget(containerWidget);

    // window props
    resize(1024, 768);
    setWindowTitle("My Browser");

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
            background-color: #36383e;
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
            min-height: 40px;
            max-height: 40px;
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
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_T), this, SLOT(addNewTab()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_W), this, [this]() {
        if (webViews.count() > 0) {
            closeTab(currentTabIndex);
        }
    });
}

void MainWindow::setupSidebar()
{
    // Create sidebar widget
    sidebarWidget = new QWidget(this);
    sidebarWidget->setObjectName("sidebarWidget");
    sidebarWidget->setFixedWidth(220);

    // Create sidebar layout
    sidebarLayout = new QVBoxLayout(sidebarWidget);
    sidebarLayout->setContentsMargins(10, 10, 10, 10);
    sidebarLayout->setSpacing(10);

    // Add a logo or title
    auto* sidebarTitle = new QLabel("Browser Sidebar", sidebarWidget);
    sidebarTitle->setAlignment(Qt::AlignCenter);
    sidebarTitle->setStyleSheet("font-size: 16px; font-weight: bold; margin-bottom: 10px;");
    sidebarLayout->addWidget(sidebarTitle);

    // Setup navigation buttons
    setupIcons();

    // Add URL display (simplified)
    urlBar = new QLineEdit(sidebarWidget);
    urlBar->setPlaceholderText("Enter URL...");
    urlBar->setObjectName("urlDisplay");
    connect(urlBar, &QLineEdit::returnPressed, this, &MainWindow::navigateToUrl);
    sidebarLayout->addWidget(urlBar);

    // Add spacing before tabs
    sidebarLayout->addSpacing(15);

    // Add a label for tabs section
    auto* tabsLabel = new QLabel("Open Tabs", sidebarWidget);
    tabsLabel->setStyleSheet("font-weight: bold;");
    sidebarLayout->addWidget(tabsLabel);

    // Create scrollable tabs container
    auto* scrollArea = new QScrollArea(sidebarWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    tabsContainer = new QWidget(scrollArea);
    tabsLayout = new QVBoxLayout(tabsContainer);
    tabsLayout->setContentsMargins(0, 5, 0, 5);
    tabsLayout->setSpacing(5);
    tabsLayout->setAlignment(Qt::AlignTop);

    scrollArea->setWidget(tabsContainer);
    sidebarLayout->addWidget(scrollArea);
}

void MainWindow::setupIcons()
{
    // Create navigation button row
    auto* navButtonLayout = new QHBoxLayout();
    navButtonLayout->setSpacing(5);

    // Navigation buttons
    backButton = new QToolButton(sidebarWidget);
    backButton->setIcon(Utils::createIconFromResource(":/icons/assets/back.svg"));
    backButton->setToolTip("Back");
    backButton->setFixedSize(32, 32);
    connect(backButton, &QToolButton::clicked, this, &MainWindow::goBack);

    forwardButton = new QToolButton(sidebarWidget);
    forwardButton->setIcon(Utils::createIconFromResource(":/icons/assets/forward.svg"));
    forwardButton->setToolTip("Forward");
    forwardButton->setFixedSize(32, 32);
    connect(forwardButton, &QToolButton::clicked, this, &MainWindow::goForward);

    reloadButton = new QToolButton(sidebarWidget);
    reloadButton->setIcon(Utils::createIconFromResource(":/icons/assets/reload.svg"));
    reloadButton->setToolTip("Reload");
    reloadButton->setFixedSize(32, 32);
    connect(reloadButton, &QToolButton::clicked, this, &MainWindow::reload);

    addTabButton = new QToolButton(sidebarWidget);
    addTabButton->setIcon(Utils::createIconFromResource(":/icons/assets/plus.svg"));
    addTabButton->setToolTip("New Tab");
    addTabButton->setFixedSize(32, 32);
    connect(addTabButton, &QToolButton::clicked, this, [this]() { addNewTab(); });

    navButtonLayout->addWidget(backButton);
    navButtonLayout->addWidget(forwardButton);
    navButtonLayout->addWidget(reloadButton);
    navButtonLayout->addWidget(addTabButton);
    navButtonLayout->addStretch(1);

    sidebarLayout->addLayout(navButtonLayout);
}

QToolButton* MainWindow::createTabButton(const QString& title, int index) {
    auto* button = new QToolButton(tabsContainer);
    button->setText(title);
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    button->setCheckable(true);
    button->setProperty("class", "TabButton");
    button->setProperty("index", index);
    // full width
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QWebEngineView* view = webViews[index];
    if (!view->icon().isNull()) {
        button->setIcon(view->icon());
    }

    auto* closeButton = new QToolButton(button);
    closeButton->setIcon(Utils::createIconFromResource(":/icons/assets/close.svg", QColor(200, 200, 200)));
    closeButton->setFixedSize(16, 16);
    closeButton->setStyleSheet("QToolButton { background: transparent; border: none; }");
    closeButton->setCursor(Qt::ArrowCursor);

    auto* layout = new QHBoxLayout(button);
    layout->setContentsMargins(5, 0, 5, 0);
    layout->setSpacing(5);
    layout->addStretch();
    layout->addWidget(closeButton);

    connect(button, &QToolButton::clicked, this, [this, index]() {
        tabClicked(index);
    });
    connect(closeButton, &QToolButton::clicked, this, [this, index]() {
        closeTab(index);
    });

    return button;
}

void MainWindow::updateTabButtons() {
    // Clear existing tab buttons
    while (QLayoutItem* item = tabsLayout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }

    // new tab buttons
    for (int i = 0; i < webViews.size(); ++i) {
        QWebEngineView* view = webViews[i];
        QString title = view->title();
        if (title.isEmpty()) {
            title = "New Tab";
        } else {
            title = Utils::truncateString(title, 20);
        }

        QToolButton* button = createTabButton(title, i);
        button->setChecked(i == currentTabIndex);
        button->setProperty("selected", i == currentTabIndex);

        tabsLayout->addWidget(button);
    }
}

void MainWindow::createWebView(const QUrl& url)
{
    auto* webView = new QWebEngineView(this);
    webView->load(url);
    webViews.append(webView);
    contentStack->addWidget(webView);

    // Set as current tab
    currentTabIndex = webViews.size() - 1;
    contentStack->setCurrentWidget(webView);

    // signal update URL bar
    connect(webView, &QWebEngineView::urlChanged, this, &MainWindow::updateUrlBar);

    // Update tab title when the page title changes
    connect(webView, &QWebEngineView::titleChanged, this, [this, webView](const QString &title) {
        int index = webViews.indexOf(webView);
        if (index != -1) {
            // Update window title for the current tab
            if (currentTabIndex == index) {
                setWindowTitle(title + " - My Browser");
            }

            // Update tab buttons
            updateTabButtons();
        }
    });

    // update favicon when it changes
    connect(webView, &QWebEngineView::iconChanged, this, [this, webView](const QIcon &icon) {
        // Force update of tab buttons when icon changes
        updateTabButtons();
    });

    // Update the sidebar tabs
    updateTabButtons();

    webView->setFocus();
}

QWebEngineView* MainWindow::currentWebView() const
{
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
    if (auto *webView = qobject_cast<QWebEngineView*>(sender()); webView == currentWebView()) {
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

void MainWindow::addNewTab(const QUrl &url)
{
    createWebView(url);
}

void MainWindow::closeTab(const int index)
{
    // keep at least one tab open
    if (webViews.count() <= 1) {
        QMessageBox::information(this, "Cannot Close Tab", "Cannot close the last tab. Application would exit instead.");
        return;
    }

    QWebEngineView* webView = webViews.takeAt(index);
    contentStack->removeWidget(webView);
    delete webView;

    // update current index
    if (currentTabIndex >= webViews.size()) {
        currentTabIndex = webViews.size() - 1;
    }

    // show the new current tab
    if (currentTabIndex >= 0 && currentTabIndex < webViews.size()) {
        contentStack->setCurrentWidget(webViews[currentTabIndex]);

        QWebEngineView* view = webViews[currentTabIndex];
        urlBar->setText(Utils::createDisplayUrl(view->url()));

        QString title = view->title();
        if (!title.isEmpty()) {
            setWindowTitle(title + " - My Browser");
        } else {
            setWindowTitle("My Browser");
        }
    }

    updateTabButtons();
}

void MainWindow::tabClicked(int index)
{
    if (index != currentTabIndex && index >= 0 && index < webViews.size()) {
        currentTabIndex = index;
        contentStack->setCurrentWidget(webViews[index]);

        // Update URL bar
        QWebEngineView* view = webViews[index];
        urlBar->setText(Utils::createDisplayUrl(view->url()));

        QString title = view->title();
        if (!title.isEmpty()) {
            setWindowTitle(title + " - My Browser");
        } else {
            setWindowTitle("My Browser");
        }

        updateTabButtons();
    }
}

// Window dragging
void MainWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && titleBar->rect().contains(event->pos())) {
        isDragging = true;
        dragStartPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    if (isDragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPos() - dragStartPosition);
        event->accept();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        isDragging = false;
        event->accept();
    }
}
