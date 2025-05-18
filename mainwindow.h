#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebEngineView>
#include <QLineEdit>

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

private:
    QWebEngineView *webView;
    QLineEdit *urlBar;
};

#endif // MAINWINDOW_H
