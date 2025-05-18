#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebEngineView>
#include <QLineEdit>
#include <QVector>
#include <QUrl>
#include <QStyleOptionTab>
#include <QPainter>
#include <QPoint>
#include <QVBoxLayout>
#include <QStackedWidget>

#include "title_bar.h"

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void navigateToUrl() const;
    void updateUrlBar(const QUrl &url) const;
    void goBack() const;
    void goForward() const;
    void reload() const;
    void addNewTab(const QUrl &url = QUrl("https://www.google.com"));
    void closeTab(int index);
    void tabClicked(int index);

private:
    QStackedWidget *contentStack;
    QVector<QWebEngineView*> webViews;
    QLineEdit *urlBar;
    QWidget *sidebarWidget;
    QVBoxLayout *sidebarLayout;
    QVBoxLayout *tabsLayout;
    QWidget *tabsContainer;
    QToolButton *backButton;
    QToolButton *forwardButton;
    QToolButton *reloadButton;
    QToolButton *addTabButton;
    TitleBar *titleBar;
    bool isDragging = false;
    QPoint dragStartPosition;
    int currentTabIndex = -1;

    void setupUI();
    void setupSidebar();
    void setupIcons();
    void createWebView(const QUrl& url = QUrl("https://www.google.com"));
    QWebEngineView* currentWebView() const;
    void updateTabButtons();
    QToolButton* createTabButton(const QString& title, int index);
};


#endif // MAINWINDOW_H
