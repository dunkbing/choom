#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include <QPoint>
#include <QStackedWidget>
#include <QVector>

#include "command_palette.h"
#include "qmlwebview.h"
#include "title_bar.h"
#include "webview_container.h"

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void goBack() const;
    void goForward() const;
    void reload() const;
    void addNewTab(const QUrl &url = QUrl("https://www.google.com"));
#ifdef Q_OS_MAC
    void *macosUrlField = nullptr; // NSTextField pointer
    // friend class MacOSTitleBar;    // Allow MacOSTitleBar to access our private members
#endif
    QmlWebView *currentWebView() const;
    void navigateToUrl() const;
    QLineEdit *urlBar;

private slots:
    void updateUrlBar(const QUrl &url) const;

    void showCommandPalette() const;
    void handleCommandPaletteUrl(const QUrl &url);
    void closeTab(int index);
    void tabClicked(int index);

private:
    QStackedWidget *contentStack;
    QVector<QmlWebView *> webViews;
    QVector<WebViewContainer *> webViewContainers;
    QWidget *sidebarWidget;
    QVBoxLayout *sidebarLayout;
    QVBoxLayout *tabsLayout;
    QWidget *tabsContainer;
    TitleBar *titleBar;
    CommandPalette *commandPalette;
    bool isDragging = false;
    QPoint dragStartPosition;
    int currentTabIndex = -1;

    void setupUI();
    void setupSidebar();
    void createWebView(const QUrl &url = QUrl("https://www.google.com"));
    void updateTabButtons();
    QToolButton *createTabButton(const QString &title, int index);
};

#endif // MAINWINDOW_H
