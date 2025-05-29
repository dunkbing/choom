//
// Created by Bùi Đặng Bình on 19/5/25.
//

#include "qmltabswidget.h"
#include <QDebug>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>

QmlTabsWidget::QmlTabsWidget(QWidget *parent) : QQuickWidget(parent) {
    engine()->addImportPath(":/qml");

    setResizeMode(QQuickWidget::SizeRootObjectToView);
    setClearColor(QColor("#24262e")); // Match sidebar background
    setAttribute(Qt::WA_OpaquePaintEvent, true);

    // Load the QML file
    setSource(QUrl("qrc:/qml/TabsList.qml"));

    // Check for errors
    if (status() != QQuickWidget::Ready) {
        qDebug() << "QML Tabs loading errors:";
        for (const QQmlError &error : errors()) {
            qDebug() << error.toString();
        }
    } else {
        qDebug() << "QML Tabs loaded successfully";
    }

    // Get root item and connect signals
    m_rootItem = rootObject();
    if (m_rootItem) {
        connect(m_rootItem, SIGNAL(tabClicked(int)), this, SLOT(handleTabClicked(int)));
        connect(m_rootItem, SIGNAL(tabCloseClicked(int)), this, SLOT(handleTabCloseClicked(int)));
        qDebug() << "QML Tabs signals connected";
    } else {
        qDebug() << "Failed to get QML root item";
    }
}

void QmlTabsWidget::addTab(const QString &title, const QString &iconUrl, bool isSelected) {
    if (m_rootItem) {
        qDebug() << "Adding tab:" << title << iconUrl << isSelected;
        bool result =
            QMetaObject::invokeMethod(m_rootItem, "addTab", Q_ARG(QVariant, title),
                                      Q_ARG(QVariant, iconUrl), Q_ARG(QVariant, isSelected));
        qDebug() << "QMetaObject::invokeMethod result:" << result;
    } else {
        qDebug() << "Cannot add tab: m_rootItem is null";
    }
}

void QmlTabsWidget::removeTab(int index) {
    if (m_rootItem) {
        QMetaObject::invokeMethod(m_rootItem, "removeTab", Q_ARG(QVariant, index));
    }
}

void QmlTabsWidget::updateTab(int index, const QString &title, const QString &iconUrl,
                              bool isSelected) {
    if (m_rootItem) {
        QMetaObject::invokeMethod(m_rootItem, "updateTab", Q_ARG(QVariant, index),
                                  Q_ARG(QVariant, title), Q_ARG(QVariant, iconUrl),
                                  Q_ARG(QVariant, isSelected));
    }
}

void QmlTabsWidget::setTabSelected(int index) {
    if (m_rootItem) {
        QMetaObject::invokeMethod(m_rootItem, "setTabSelected", Q_ARG(QVariant, index),
                                  Q_ARG(QVariant, true));
    }
}

void QmlTabsWidget::clearTabs() {
    if (m_rootItem) {
        qDebug() << "Clearing tabs";
        bool result = QMetaObject::invokeMethod(m_rootItem, "clearTabs");
        qDebug() << "clearTabs QMetaObject::invokeMethod result:" << result;
    } else {
        qDebug() << "Cannot clear tabs: m_rootItem is null";
    }
}

void QmlTabsWidget::handleTabClicked(int index) {
    qDebug() << "Tab clicked:" << index;
    emit tabClicked(index);
}

void QmlTabsWidget::handleTabCloseClicked(int index) {
    qDebug() << "Tab close clicked:" << index;
    emit tabCloseClicked(index);
}
