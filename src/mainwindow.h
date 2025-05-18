#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebEngineView>
#include <QLineEdit>
#include <QComboBox>
#include <QVector>
#include <QUrl>
#include <QStyleOptionTab>
#include <QPainter>
#include <QPoint>

#include "tab.h"

class CustomTitleBar;

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
    void changeTabOrientation(int orientation);
    void tabChanged(int index);

private:
    TabWidget *tabWidget;
    QLineEdit *urlBar;
    QToolBar *navigationBar;
    QComboBox *tabOrientationSelector;
    QAction *addTabAction;
    CustomTitleBar *titleBar;
    int currentTabOrientation = Qt::Horizontal;
    bool isDragging = false;
    QPoint dragStartPosition;

    void setupUI();
    void setupIcons();
    void createWebView(const QUrl& url = QUrl("https://www.google.com"));
    QWebEngineView* currentWebView() const;
};

class CustomTitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit CustomTitleBar(QWidget *parent = nullptr);

    signals:
        void minimizeClicked();
    void maximizeClicked();
    void closeClicked();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QAction *minimizeAction;
    QAction *maximizeAction;
    QAction *closeAction;
};

#endif // MAINWINDOW_H
