//
// Created by Bùi Đặng Bình on 19/5/25.
//

#ifndef QMLWEBVIEW_H
#define QMLWEBVIEW_H

#include <QIcon>
#include <QQuickItem>
#include <QQuickWidget>
#include <QUrl>

class QmlWebView : public QQuickWidget {
    Q_OBJECT

public:
    explicit QmlWebView(QWidget *parent = nullptr);

    void load(const QUrl &url);
    QUrl url() const;
    QString title() const;
    QIcon icon() const;
    QString faviconUrl() const; // Add this method

    void back();
    void forward();
    void reload();
    void stop();

signals:
    void urlChanged(const QUrl &url);
    void titleChanged(const QString &title);
    void iconChanged(const QIcon &icon);

private slots:
    void handleUrlChanged(const QString &url);
    void handleTitleChanged(const QString &title);
    void handleIconChanged(const QVariant &icon);

private:
    QQuickItem *m_rootItem;
    QQuickItem *m_webView;
    QUrl m_currentUrl;
    QString m_currentTitle;
    QIcon m_currentIcon;
    QString m_faviconUrl; // Add this member
};

#endif // QMLWEBVIEW_H
