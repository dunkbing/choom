#include "mainwindow.h"
#include <QHBoxLayout>
#include <QAction>
#include <QToolBar>
#include <QMenu>
#include <QLabel>
#include <QApplication>
#include <QMessageBox>
#include <QShortcut>
#include <QStylePainter>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    addNewTab();
}

void MainWindow::setupUI()
{
    // Create the custom tab widget
    tabWidget = new TabWidget(this);
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);
    connect(tabWidget, &QTabWidget::currentChanged, this, &MainWindow::tabChanged);

    // URL bar
    urlBar = new QLineEdit(this);
    connect(urlBar, &QLineEdit::returnPressed, this, &MainWindow::navigateToUrl);

    // toolbar
    navigationBar = new QToolBar(this);
    addToolBar(navigationBar);

    // navigation buttons
    const QAction *backAction = navigationBar->addAction("Back");
    connect(backAction, &QAction::triggered, this, &MainWindow::goBack);

    const QAction *forwardAction = navigationBar->addAction("Forward");
    connect(forwardAction, &QAction::triggered, this, &MainWindow::goForward);

    const QAction *reloadAction = navigationBar->addAction("Reload");
    connect(reloadAction, &QAction::triggered, this, &MainWindow::reload);

    navigationBar->addWidget(urlBar);

    // new tab button
    addTabAction = navigationBar->addAction("New Tab");
    connect(addTabAction, &QAction::triggered, this, [this]() { addNewTab(); });

    // tab orientation selector
    auto *orientationLabel = new QLabel("Tab Orientation: ");
    navigationBar->addWidget(orientationLabel);

    tabOrientationSelector = new QComboBox(this);
    tabOrientationSelector->addItem("Horizontal", Qt::Horizontal);
    tabOrientationSelector->addItem("Vertical", Qt::Vertical);
    connect(tabOrientationSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::changeTabOrientation);
    navigationBar->addWidget(tabOrientationSelector);

    setCentralWidget(tabWidget);

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
        }
        QLineEdit {
            background-color: #36383e;
            color: #ffffff;
            border: 1px solid #444;
            padding: 5px;
            border-radius: 2px;
        }
        QComboBox {
            background-color: #36383e;
            color: #ffffff;
            border: 1px solid #444;
            padding: 5px;
        }
        QTabWidget {
            background-color: #24262e;
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
