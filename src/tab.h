//
// Created by Bùi Đặng Bình on 18/5/25.
//

#ifndef TAB_H
#define TAB_H

#include <QTabWidget>
#include <QTabBar>

class TabBar : public QTabBar
{
    Q_OBJECT
public:
    explicit TabBar(QWidget *parent = nullptr);

protected:
    QSize tabSizeHint(int index) const override;
    void paintEvent(QPaintEvent *event) override;
    bool isVertical() const;
};

class TabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit TabWidget(QWidget *parent = nullptr);
    void setTabBarOrientation(Qt::Orientation orientation);

private:
    TabBar *customTabBar;
};

#endif //TAB_H
