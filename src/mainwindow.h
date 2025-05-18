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

#include "tab.h"

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

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
    int currentTabOrientation = Qt::Horizontal;

    void setupUI();
    void createWebView(const QUrl& url = QUrl("https://www.google.com"));
    QWebEngineView* currentWebView() const;
};

#endif // MAINWINDOW_H
