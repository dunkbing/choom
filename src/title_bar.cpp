//
// Created by Bùi Đặng Bình on 18/5/25.
//

#include "title_bar.h"

#include <QBoxLayout>
#include <QStylePainter>
#include <QToolButton>

// CustomTitleBar implementation
TitleBar::TitleBar(QWidget *parent) : QWidget(parent) {
    setFixedHeight(30);

    // Create layout
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 0, 5, 0);
    layout->setSpacing(0);

    // Add spacer to push controls to the right
    layout->addStretch();

    // window control buttons
    auto* minimizeButton = new QToolButton(this);
    auto* maximizeButton = new QToolButton(this);
    auto* closeButton = new QToolButton(this);

    minimizeButton->setFixedSize(30, 30);
    maximizeButton->setFixedSize(30, 30);
    closeButton->setFixedSize(30, 30);

    minimizeButton->setText("🗕");
    maximizeButton->setText("🗖");
    closeButton->setText("✕");

    minimizeButton->setStyleSheet("QToolButton { border: none; color: #aaa; } QToolButton:hover { background-color: #444; }");
    maximizeButton->setStyleSheet("QToolButton { border: none; color: #aaa; } QToolButton:hover { background-color: #444; }");
    closeButton->setStyleSheet("QToolButton { border: none; color: #aaa; } QToolButton:hover { background-color: #e81123; color: white; }");

    connect(minimizeButton, &QToolButton::clicked, this, &TitleBar::minimizeClicked);
    connect(maximizeButton, &QToolButton::clicked, this, &TitleBar::maximizeClicked);
    connect(closeButton, &QToolButton::clicked, this, &TitleBar::closeClicked);

    layout->addWidget(minimizeButton);
    layout->addWidget(maximizeButton);
    layout->addWidget(closeButton);

    setStyleSheet("background-color: #24262e;");
}

void TitleBar::paintEvent(QPaintEvent *) {
    QStylePainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(36, 38, 46));
    painter.drawRect(rect());
}