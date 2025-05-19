//
// Created by Bùi Đặng Bình on 19/5/25.
//

#ifndef WEBVIEW_CONTAINER_H
#define WEBVIEW_CONTAINER_H

#include <QWebEngineView>
#include <QWidget>
#include <QVBoxLayout>
#include <QFrame>

class WebViewContainer : public QFrame
{
    Q_OBJECT

public:
    explicit WebViewContainer(QWebEngineView* webView, QWidget* parent = nullptr);
    QWebEngineView* webView() const;

private:
    QWebEngineView* m_webView;
    QVBoxLayout* m_layout;
};

#endif // WEBVIEW_CONTAINER_H
