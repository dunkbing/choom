#include "mainwindow.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QApplication>
#include <QMessageBox>
#include <QShortcut>
#include <QStylePainter>
#include <QMouseEvent>
#include <QToolButton>
#include <QSvgRenderer>
#include <QPainter>
#include <QStyle>
#include <QScrollArea>

static const char* backIconSvg = R"(
<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
  <path d="M19 12H5M12 19l-7-7 7-7"/>
</svg>
)";

static const char* forwardIconSvg = R"(
<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
  <path d="M5 12h14M12 5l7 7-7 7"/>
</svg>
)";

static const char* reloadIconSvg = R"(
<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
  <path d="M3 12a9 9 0 1 0 9-9 9.75 9.75 0 0 0-6.74 2.74L3 8"/>
  <path d="M3 3v5h5"/>
</svg>
)";

static const char* newTabIconSvg = R"(
<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
  <rect x="3" y="3" width="18" height="18" rx="2" ry="2"/>
  <line x1="12" y1="8" x2="12" y2="16"/>
  <line x1="8" y1="12" x2="16" y2="12"/>
</svg>
)";

static const char* closeIconSvg = R"(
<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
  <line x1="18" y1="6" x2="6" y2="18"/>
  <line x1="6" y1="6" x2="18" y2="18"/>
</svg>
)";

QIcon createIconFromSvg(const QString& svgData, const QColor& color = Qt::white) {
    QSvgRenderer renderer(svgData.toUtf8());
    QPixmap pixmap(24, 24);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setPen(color);
    renderer.render(&painter);
    return QIcon(pixmap);
}

// CustomTitleBar implementation
CustomTitleBar::CustomTitleBar(QWidget *parent) : QWidget(parent) {
    setFixedHeight(30);

    // Create layout
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 0, 5, 0);
    layout->setSpacing(0);

    // Add spacer to push controls to the right
    layout->addStretch();

    // window control buttons
    auto* minimizeButton = new QToolButton(this);
    auto* maximizeButton = new QToolButton(this);
    auto* closeButton = new QToolButton(this);

    minimizeButton->setFixedSize(30, 30);
    maximizeButton->setFixedSize(30, 30);
    closeButton->setFixedSize(30, 30);

    minimizeButton->setText("🗕");
    maximizeButton->setText("🗖");
    closeButton->setText("✕");

    minimizeButton->setStyleSheet("QToolButton { border: none; color: #aaa; } QToolButton:hover { background-color: #444; }");
    maximizeButton->setStyleSheet("QToolButton { border: none; color: #aaa; } QToolButton:hover { background-color: #444; }");
    closeButton->setStyleSheet("QToolButton { border: none; color: #aaa; } QToolButton:hover { background-color: #e81123; color: white; }");

    connect(minimizeButton, &QToolButton::clicked, this, &CustomTitleBar::minimizeClicked);
    connect(maximizeButton, &QToolButton::clicked, this, &CustomTitleBar::maximizeClicked);
    connect(closeButton, &QToolButton::clicked, this, &CustomTitleBar::closeClicked);

    layout->addWidget(minimizeButton);
    layout->addWidget(maximizeButton);
    layout->addWidget(closeButton);

    setStyleSheet("background-color: #24262e;");
}

void CustomTitleBar::paintEvent(QPaintEvent *) {
    QStylePainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(36, 38, 46));
    painter.drawRect(rect());
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), isDragging(false)
{
    setupUI();
    addNewTab();
}

void MainWindow::setupUI()
{
    // Create title bar
    titleBar = new CustomTitleBar(this);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    setAttribute(Qt::WA_TranslucentBackground);
    connect(titleBar, &CustomTitleBar::minimizeClicked, this, &MainWindow::showMinimized);
    connect(titleBar, &CustomTitleBar::maximizeClicked, this, [this]() {
        if (isMaximized()) {
            showNormal();
        } else {
            showMaximized();
        }
    });
    connect(titleBar, &CustomTitleBar::closeClicked, this, &MainWindow::close);

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
    backButton->setIcon(createIconFromSvg(backIconSvg));
    backButton->setToolTip("Back");
    backButton->setFixedSize(32, 32);
    connect(backButton, &QToolButton::clicked, this, &MainWindow::goBack);

    forwardButton = new QToolButton(sidebarWidget);
    forwardButton->setIcon(createIconFromSvg(forwardIconSvg));
    forwardButton->setToolTip("Forward");
    forwardButton->setFixedSize(32, 32);
    connect(forwardButton, &QToolButton::clicked, this, &MainWindow::goForward);

    reloadButton = new QToolButton(sidebarWidget);
    reloadButton->setIcon(createIconFromSvg(reloadIconSvg));
    reloadButton->setToolTip("Reload");
    reloadButton->setFixedSize(32, 32);
    connect(reloadButton, &QToolButton::clicked, this, &MainWindow::reload);

    addTabButton = new QToolButton(sidebarWidget);
    addTabButton->setIcon(createIconFromSvg(newTabIconSvg));
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
    closeButton->setIcon(createIconFromSvg(closeIconSvg, QColor(200, 200, 200)));
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
        } else if (title.length() > 20) {
            title = title.left(17) + "...";
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

    QString url = urlBar->text();
    if (!url.contains("://")) {
        url = "https://" + url;
    }

    webView->load(QUrl(url));
}

void MainWindow::updateUrlBar(const QUrl &url) const {
    // Only update if the sender is the current tab
    if (auto *webView = qobject_cast<QWebEngineView*>(sender()); webView == currentWebView()) {
        // Simplify the URL display
        QString displayUrl = url.host();
        if (displayUrl.isEmpty()) {
            displayUrl = url.toString();
        } else {
            // Add path if it's not just "/"
            if (url.path().length() > 1) {
                displayUrl += url.path();
            }
        }
        urlBar->setText(displayUrl);
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

    // Remove the web view
    QWebEngineView* webView = webViews.takeAt(index);
    contentStack->removeWidget(webView);
    delete webView;

    // Update current index if needed
    if (currentTabIndex >= webViews.size()) {
        currentTabIndex = webViews.size() - 1;
    }

    // Show the new current tab
    if (currentTabIndex >= 0 && currentTabIndex < webViews.size()) {
        contentStack->setCurrentWidget(webViews[currentTabIndex]);

        // Update URL and title
        QWebEngineView* view = webViews[currentTabIndex];
        QString displayUrl = view->url().host();
        if (displayUrl.isEmpty()) {
            displayUrl = view->url().toString();
        } else if (view->url().path().length() > 1) {
            displayUrl += view->url().path();
        }
        urlBar->setText(displayUrl);

        QString title = view->title();
        if (!title.isEmpty()) {
            setWindowTitle(title + " - My Browser");
        } else {
            setWindowTitle("My Browser");
        }
    }

    // Update tab buttons
    updateTabButtons();
}

void MainWindow::tabClicked(int index)
{
    if (index != currentTabIndex && index >= 0 && index < webViews.size()) {
        currentTabIndex = index;
        contentStack->setCurrentWidget(webViews[index]);

        // Update URL bar
        QWebEngineView* view = webViews[index];
        QString displayUrl = view->url().host();
        if (displayUrl.isEmpty()) {
            displayUrl = view->url().toString();
        } else if (view->url().path().length() > 1) {
            displayUrl += view->url().path();
        }
        urlBar->setText(displayUrl);

        // Update window title
        QString title = view->title();
        if (!title.isEmpty()) {
            setWindowTitle(title + " - My Browser");
        } else {
            setWindowTitle("My Browser");
        }

        // Update tab buttons
        updateTabButtons();
    }
}

void MainWindow::tabChanged(const int index)
{
    // This function is now handled by tabClicked
}

// Window dragging functionality
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
