#include "qmlwebview.h"
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>

QmlWebView::QmlWebView(QWidget *parent) : QQuickWidget(parent) {
    engine()->addImportPath(":/qml");

    setResizeMode(QQuickWidget::SizeRootObjectToView);
    setClearColor(Qt::transparent);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    setAttribute(Qt::WA_NoSystemBackground, false);

    // Load the QML file from resources
    setSource(QUrl("qrc:/qml/RoundedWebView.qml"));

    // Check for errors
    if (status() != QQuickWidget::Ready) {
        qDebug() << "QML loading errors:";
        for (const QQmlError &error : errors()) {
            qDebug() << error.toString();
        }
    } else {
        qDebug() << "QML loaded successfully";
    }

    // Get references to QML objects
    m_rootItem = rootObject();
    if (m_rootItem) {
        // Connect signals with corrected signal names
        connect(m_rootItem, SIGNAL(webUrlChanged(QString)), this, SLOT(handleUrlChanged(QString)));
        connect(m_rootItem, SIGNAL(webTitleChanged(QString)), this,
                SLOT(handleTitleChanged(QString)));
        connect(m_rootItem, SIGNAL(webIconChanged(QVariant)), this,
                SLOT(handleIconChanged(QVariant)));

        // Get WebView reference
        m_webView = m_rootItem->property("webView").value<QQuickItem *>();
    }
}

void QmlWebView::load(const QUrl &url) {
    qDebug() << "QmlWebView::load called with URL:" << url.toString();
    if (m_rootItem) {
        QMetaObject::invokeMethod(m_rootItem, "load", Q_ARG(QVariant, url.toString()));
    } else {
        qDebug() << "Error: rootItem is null";
    }
}

QUrl QmlWebView::url() const {
    return m_currentUrl;
}

QString QmlWebView::title() const {
    return m_currentTitle;
}

QIcon QmlWebView::icon() const {
    return m_currentIcon;
}

void QmlWebView::back() {
    if (m_rootItem) {
        QMetaObject::invokeMethod(m_rootItem, "back");
    }
}

void QmlWebView::forward() {
    if (m_rootItem) {
        QMetaObject::invokeMethod(m_rootItem, "forward");
    }
}

void QmlWebView::reload() {
    if (m_rootItem) {
        QMetaObject::invokeMethod(m_rootItem, "reload");
    }
}

void QmlWebView::stop() {
    if (m_rootItem) {
        QMetaObject::invokeMethod(m_rootItem, "stop");
    }
}

void QmlWebView::handleUrlChanged(const QString &url) {
    m_currentUrl = QUrl(url);
    emit urlChanged(m_currentUrl);
}

void QmlWebView::handleTitleChanged(const QString &title) {
    m_currentTitle = title;
    emit titleChanged(title);
}

void QmlWebView::handleIconChanged(const QVariant &icon) {
    // Convert QVariant to QIcon (this is simplified - you might need more conversion)
    m_currentIcon = QIcon(); // In a real app, convert from QVariant
    emit iconChanged(m_currentIcon);
}
