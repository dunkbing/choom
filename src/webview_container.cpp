//
// Created by Bùi Đặng Bình on 19/5/25.
//

#include "webview_container.h"
#include "qmlwebview.h"

WebViewContainer::WebViewContainer(QmlWebView *webView, QWidget *parent)
    : QFrame(parent), m_webView(webView) {
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(8, 8, 8, 8);
    m_layout->setSpacing(0);

    m_layout->addWidget(m_webView);

    qDebug() << "WebViewContainer created";
}

QmlWebView *WebViewContainer::webView() const {
    return m_webView;
}
