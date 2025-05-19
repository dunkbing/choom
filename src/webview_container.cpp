//
// Created by Bùi Đặng Bình on 19/5/25.
//

#include "webview_container.h"

WebViewContainer::WebViewContainer(QWebEngineView* webView, QWidget* parent)
    : QFrame(parent)
    , m_webView(webView)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    // Take ownership of the webView
    if (webView->parent() != this) {
        webView->setParent(this);
    }

    // Add the webView to layout
    m_layout->addWidget(webView);
}

QWebEngineView* WebViewContainer::webView() const
{
    return m_webView;
}
