#include "ui/welcome_widget.h"

WelcomeWidget::WelcomeWidget(QWidget *parent)
    : QWidget(parent) {
    setupUI();
}

void WelcomeWidget::setupUI() {
    auto *layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);

    // Main instruction label
    auto *titleLabel = new QLabel("Welcome to Database Client", this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: #ffffff; margin-bottom: 20px;");

    // Instructions
    auto *instructionsLabel = new QLabel(
        "Get started:\n\n"
        "• Click \"New Connection\" to add a database\n"
        "• Double-click a table to view its data\n"
        "• Right-click a database to open SQL Editor\n\n"
        "Keyboard Shortcuts:\n"
        "• Ctrl+T (Cmd+T) - New Connection\n"
        "• Ctrl+Enter (Cmd+Return) - Execute Query in SQL Editor",
        this
    );
    instructionsLabel->setAlignment(Qt::AlignCenter);
    instructionsLabel->setWordWrap(true);
    instructionsLabel->setStyleSheet(
        "color: #cccccc; "
        "font-size: 14px; "
        "line-height: 1.6;"
    );

    layout->addStretch();
    layout->addWidget(titleLabel);
    layout->addSpacing(20);
    layout->addWidget(instructionsLabel);
    layout->addStretch();
}
