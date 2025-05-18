//
// Created by Bùi Đặng Bình on 18/5/25.
//

#include "tab.h"

#include <QStyleOptionTab>
#include <QStylePainter>

TabBar::TabBar(QWidget *parent) : QTabBar(parent)
{
    setDrawBase(false);
    setElideMode(Qt::ElideRight);
    setExpanding(false);
    setDocumentMode(true);
}

QSize TabBar::tabSizeHint(const int index) const
{
    QSize size = QTabBar::tabSizeHint(index);

    if (isVertical()) {
        size.setWidth(200);
        size.setHeight(40);
        return size;
    }

    return size;
}

void TabBar::paintEvent(QPaintEvent *event)
{
    QStylePainter painter(this);
    QStyleOptionTab opt;

    // draw each tab with custom styling
    for (int i = 0; i < count(); i++) {
        initStyleOption(&opt, i);

        if (isVertical()) {
            QRect rect = tabRect(i);

            // Set background color based on selected state
            if (i == currentIndex()) {
                painter.fillRect(rect, QColor(60, 63, 77));  // Dark blue-gray when selected
            } else {
                painter.fillRect(rect, QColor(36, 38, 46));  // Darker background for unselected
            }

            // Draw icon
            if (!tabIcon(i).isNull()) {
                QIcon icon = tabIcon(i);
                QRect iconRect = rect;
                iconRect.setSize(QSize(16, 16));
                iconRect.moveLeft(rect.left() + 10);
                iconRect.moveTop(rect.top() + (rect.height() - 16) / 2);
                icon.paint(&painter, iconRect);
            }

            // Draw text
            QRect textRect = rect;
            textRect.setLeft(rect.left() + 36);  // Leave space for icon

            painter.setPen(i == currentIndex() ? Qt::white : QColor(200, 200, 200));
            painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, tabText(i));

            // Don't use the default drawing
            continue;
        }

        // Use default drawing for horizontal tabs
        painter.drawControl(QStyle::CE_TabBarTab, opt);
    }
}

bool TabBar::isVertical() const {
    return shape() == QTabBar::RoundedWest || shape() == QTabBar::RoundedEast;
}

// CustomTabWidget implementation
TabWidget::TabWidget(QWidget *parent) : QTabWidget(parent)
{
    customTabBar = new TabBar(this);
    setTabBar(customTabBar);

    // Set default styles
    setDocumentMode(true);
    setTabsClosable(true);
    setMovable(true);
}

void TabWidget::setTabBarOrientation(const Qt::Orientation orientation)
{
    if (orientation == Qt::Vertical) {
        setTabPosition(West);
        setStyleSheet(R"(
            QTabWidget::pane {
                border: none;
                background: #24262e;
            }
            QTabBar::tab {
                padding: 8px;
                color: #cccccc;
            }
            QTabBar::tab:selected {
                color: white;
            }
        )");
    } else {
        setTabPosition(North);
        setStyleSheet("");
    }
}
