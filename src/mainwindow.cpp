#include "mainwindow.h"
#include <QVBoxLayout>
#include <QAction>
#include <QToolBar>
#include <QMenu>
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
    auto* containerLayout = new QVBoxLayout(containerWidget);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);
    containerLayout->addWidget(titleBar);

    // Create the custom tab widget
    tabWidget = new TabWidget(this);
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);
    connect(tabWidget, &QTabWidget::currentChanged, this, &MainWindow::tabChanged);

    // URL bar
    urlBar = new QLineEdit(this);
    urlBar->setPlaceholderText("Enter URL...");
    connect(urlBar, &QLineEdit::returnPressed, this, &MainWindow::navigateToUrl);

    // toolbar
    navigationBar = new QToolBar(this);
    navigationBar->setIconSize(QSize(20, 20));
    navigationBar->setMovable(false);

    // Add actions with icons
    setupIcons();

    // Complete the container layout
    containerLayout->addWidget(navigationBar);
    containerLayout->addWidget(tabWidget, 1);
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
        QToolBar {
            background-color: #24262e;
            border: none;
            spacing: 5px;
            padding: 5px;
        }
        QLineEdit {
            background-color: #36383e;
            color: #ffffff;
            border: 1px solid #444;
            padding: 8px;
            border-radius: 4px;
            selection-background-color: #4d78cc;
        }
        QComboBox {
            background-color: #36383e;
            color: #ffffff;
            border: 1px solid #444;
            padding: 5px;
            border-radius: 4px;
            min-height: 25px;
        }
        QComboBox::drop-down {
            border: none;
            width: 20px;
        }
        QComboBox::down-arrow {
            width: 14px;
            height: 14px;
        }
        QToolButton {
            background-color: #36383e;
            border: none;
            border-radius: 4px;
            padding: 5px;
        }
        QToolButton:hover {
            background-color: #454750;
        }
        QToolButton:pressed {
            background-color: #303238;
        }
    )";

    setStyleSheet(darkStyle);

    // shortcuts
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_T), this, SLOT(addNewTab()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_W), this, [this]() {
        if (tabWidget->count() > 0) {
            closeTab(tabWidget->currentIndex());
        }
    });

    // default vertical tab
    tabOrientationSelector->setCurrentIndex(1);
}

void MainWindow::setupIcons()
{
    // action icons
    QAction *backAction = navigationBar->addAction(createIconFromSvg(backIconSvg), "Back");
    connect(backAction, &QAction::triggered, this, &MainWindow::goBack);

    QAction *forwardAction = navigationBar->addAction(createIconFromSvg(forwardIconSvg), "Forward");
    connect(forwardAction, &QAction::triggered, this, &MainWindow::goForward);

    QAction *reloadAction = navigationBar->addAction(createIconFromSvg(reloadIconSvg), "Reload");
    connect(reloadAction, &QAction::triggered, this, &MainWindow::reload);

    navigationBar->addSeparator();
    navigationBar->addWidget(urlBar);

    // new tab
    addTabAction = navigationBar->addAction(createIconFromSvg(newTabIconSvg), "New Tab");
    connect(addTabAction, &QAction::triggered, this, [this]() { addNewTab(); });

    // tab orientation selector
    auto *orientationLabel = new QLabel("Tab Orientation: ");
    orientationLabel->setStyleSheet("margin-left: 5px;");
    navigationBar->addWidget(orientationLabel);

    tabOrientationSelector = new QComboBox(this);
    tabOrientationSelector->addItem("Horizontal", Qt::Horizontal);
    tabOrientationSelector->addItem("Vertical", Qt::Vertical);
    connect(tabOrientationSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::changeTabOrientation);
    navigationBar->addWidget(tabOrientationSelector);
}

void MainWindow::createWebView(const QUrl& url)
{
    auto *webView = new QWebEngineView(this);
    webView->load(url);

    // signal update URL bar
    connect(webView, &QWebEngineView::urlChanged, this, &MainWindow::updateUrlBar);

    // add the web view to the tab widget
    const int index = tabWidget->addTab(webView, "New Tab");
    tabWidget->setCurrentIndex(index);

    // Update tab title when the page title changes
    connect(webView, &QWebEngineView::titleChanged, this, [this, webView](const QString &title) {
        if (const int index_ = tabWidget->indexOf(webView); index_ != -1) {
            QString tabTitle = title;
            if (tabTitle.isEmpty()) {
                tabTitle = "New Tab";
            } else if (tabTitle.length() > 20) {
                tabTitle = tabTitle.left(17) + "...";
            }
            tabWidget->setTabText(index_, tabTitle);

            // Update window title for the current tab
            if (tabWidget->currentIndex() == index_) {
                setWindowTitle(title + " - My Browser");
            }
        }
    });

    // update favicon when it changes
    connect(webView, &QWebEngineView::iconChanged, this, [this, webView](const QIcon &icon) {
        if (const int index_ = tabWidget->indexOf(webView); index_ != -1) {
            tabWidget->setTabIcon(index_, icon);
        }
    });

    webView->setFocus();
}

QWebEngineView* MainWindow::currentWebView() const
{
    if (tabWidget->count() == 0) {
        return nullptr;
    }
    return qobject_cast<QWebEngineView*>(tabWidget->currentWidget());
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
        urlBar->setText(url.toString());
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
    if (tabWidget->count() <= 1) {
        QMessageBox::information(this, "Cannot Close Tab", "Cannot close the last tab. Application would exit instead.");
        return;
    }

    const QWidget *widget = tabWidget->widget(index);
    tabWidget->removeTab(index);

    delete widget;
}

void MainWindow::changeTabOrientation(int index)
{
    int orientation = tabOrientationSelector->itemData(index).toInt();
    currentTabOrientation = orientation;
    tabWidget->setTabBarOrientation(static_cast<Qt::Orientation>(orientation));
}

void MainWindow::tabChanged(const int index)
{
    if (index == -1) {
        // No tabs left
        urlBar->clear();
        setWindowTitle("My Browser");
        return;
    }

    // Update URL bar for the newly selected tab
    auto *webView = qobject_cast<QWebEngineView*>(tabWidget->widget(index));
    if (webView) {
        urlBar->setText(webView->url().toString());

        // Update window title
        QString title = webView->title();
        if (!title.isEmpty()) {
            setWindowTitle(title + " - My Browser");
        } else {
            setWindowTitle("My Browser");
        }
    }
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
