//
// Created by Bùi Đặng Bình on 19/5/25.
//

#ifndef WEBVIEW_CONTAINER_H
#define WEBVIEW_CONTAINER_H

#include "qmlwebview.h"
#include <QFrame>
#include <QVBoxLayout>
#include <QWidget>

class WebViewContainer : public QFrame {
    Q_OBJECT

public:
    explicit WebViewContainer(QmlWebView *webView, QWidget *parent = nullptr);
    QmlWebView *webView() const;

private:
    QmlWebView *m_webView;
    QVBoxLayout *m_layout;
};

#endif // WEBVIEW_CONTAINER_H
