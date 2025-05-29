//
// Created by Bùi Đặng Bình on 29/5/25.
//

#ifndef QMLTABSWIDGET_H
#define QMLTABSWIDGET_H

#include <QQuickItem>
#include <QQuickWidget>

class QmlTabsWidget : public QQuickWidget {
    Q_OBJECT

public:
    explicit QmlTabsWidget(QWidget *parent = nullptr);

    void addTab(const QString &title, const QString &iconUrl = "", bool isSelected = false);
    void removeTab(int index);
    void updateTab(int index, const QString &title, const QString &iconUrl, bool isSelected);
    void setTabSelected(int index);
    void clearTabs();

signals:
    void tabClicked(int index);
    void tabCloseClicked(int index);

private slots:
    void handleTabClicked(int index);
    void handleTabCloseClicked(int index);

private:
    QQuickItem *m_rootItem;
};

#endif // QMLTABSWIDGET_H
