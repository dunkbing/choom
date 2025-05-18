#include "mainwindow.h"
#include <QVBoxLayout>
#include <QAction>
#include <QToolBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Create the web view
    webView = new QWebEngineView(this);
    webView->load(QUrl("https://www.google.com"));

    // Create URL bar
    urlBar = new QLineEdit(this);
    connect(urlBar, &QLineEdit::returnPressed, this, &MainWindow::navigateToUrl);
    connect(webView, &QWebEngineView::urlChanged, this, &MainWindow::updateUrlBar);

    // Create toolbar
    auto *navigationBar = new QToolBar(this);
    addToolBar(navigationBar);

    // Add navigation buttons
    const QAction *backAction = navigationBar->addAction("Back");
    connect(backAction, &QAction::triggered, this, &MainWindow::goBack);

    const QAction *forwardAction = navigationBar->addAction("Forward");
    connect(forwardAction, &QAction::triggered, this, &MainWindow::goForward);

    const QAction *reloadAction = navigationBar->addAction("Reload");
    connect(reloadAction, &QAction::triggered, this, &MainWindow::reload);

    navigationBar->addWidget(urlBar);

    // Set up the layout
    auto *centralWidget = new QWidget(this);
    auto *layout = new QVBoxLayout(centralWidget);
    layout->addWidget(webView);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    // Set window properties
    resize(1024, 768);
    setWindowTitle("My Browser");
}

void MainWindow::navigateToUrl() const {
    QString url = urlBar->text();

    if (!url.contains("://")) {
        url = "https://" + url;
    }

    webView->load(QUrl(url));
}

void MainWindow::updateUrlBar(const QUrl &url) const {
    urlBar->setText(url.toString());
}

void MainWindow::goBack() const {
    webView->back();
}

void MainWindow::goForward() const {
    webView->forward();
}

void MainWindow::reload() const {
    webView->reload();
}
