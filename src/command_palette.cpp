//
// Created by Bùi Đặng Bình on 18/5/25.
//

#include "command_palette.h"
#include "utils.h"

#include <QApplication>
#include <QDebug>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QShowEvent>

CommandPalette::CommandPalette(QWidget *parent) : QDialog(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);

    initializeUI();
    initializeWebsites();

    // Connect signals
    connect(searchBox, &QLineEdit::textChanged, this, &CommandPalette::filterWebsites);
    connect(websiteList, &QListWidget::itemClicked, this, &CommandPalette::selectWebsite);
    connect(websiteList, &QListWidget::itemActivated, this, &CommandPalette::itemActivated);

    // Install event filter to handle keyboard navigation
    searchBox->installEventFilter(this);
    websiteList->installEventFilter(this);

    // Initialize with all websites showing
    updateListFromFilter("");
}

CommandPalette::~CommandPalette() = default;

void CommandPalette::initializeUI() {
    // Set up size
    resize(600, 400);

    // Create main layout
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(10);

    // Add search box
    searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText("Search with Google or enter address");
    searchBox->setObjectName("commandPaletteSearch");

    // Add websites list
    websiteList = new QListWidget(this);
    websiteList->setObjectName("commandPaletteList");
    websiteList->setFrameShape(QFrame::NoFrame);
    websiteList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    websiteList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    websiteList->setSpacing(4); // Add spacing between items

    // Apply styling
    setStyleSheet(R"(
        CommandPalette {
            background-color: #24262e;
            border: 1px solid #444;
            border-radius: 8px;
        }
        QLineEdit#commandPaletteSearch {
            background-color: #36383e;
            color: #ffffff;
            border: 1px solid #444;
            padding: 12px;
            border-radius: 6px;
            font-size: 14px;
            selection-background-color: #4d78cc;
        }
        QListWidget#commandPaletteList {
            background-color: #24262e;
            color: #ffffff;
            border: none;
            outline: none;
            font-size: 13px;
        }
        QListWidget#commandPaletteList::item {
            background-color: transparent;
            border-radius: 6px;
            padding: 0px;
        }
        QListWidget#commandPaletteList::item:selected, QListWidget#commandPaletteList::item:hover {
            background-color: #36383e;
        }
    )");

    layout->addWidget(searchBox);
    layout->addWidget(websiteList);
}

void CommandPalette::initializeWebsites() {
    websites["notion"] = Website("Notion", "notion.com", ":/icons/assets/notion.png");
    websites["x"] = Website("X", "x.com/zen_browser", ":/icons/assets/x.png");
    websites["reddit"] = Website("Reddit", "reddit.com/r/zen_browser", ":/icons/assets/reddit.png");
}

void CommandPalette::addWebsiteToList(const Website &website) const {
    auto item = new QListWidgetItem(websiteList);

    // Create a widget to hold the icon and text
    auto itemWidget = new QWidget();
    itemWidget->setObjectName("paletteItemWidget");
    itemWidget->setAttribute(Qt::WA_TranslucentBackground); // Make widget background transparent

    auto itemLayout = new QHBoxLayout(itemWidget);
    itemLayout->setContentsMargins(10, 8, 10, 8);
    itemLayout->setSpacing(10);

    // Create icon label
    auto iconLabel = new QLabel();
    iconLabel->setFixedSize(24, 24);
    iconLabel->setAlignment(Qt::AlignCenter);

    // Try to load the icon
    if (!website.icon.isEmpty()) {
        QIcon icon = Utils::createIconFromResource(website.icon);
        if (!icon.isNull()) {
            iconLabel->setPixmap(icon.pixmap(24, 24));
        }
    }

    // Create name and URL labels
    auto nameLabel = new QLabel(website.name);
    nameLabel->setStyleSheet("font-weight: bold; color: white;");

    auto urlLabel = new QLabel();
    QString displayUrl;

    if (website.url.contains("://")) {
        displayUrl = website.url;
    } else {
        displayUrl = website.url;
    }

    // Add an em dash between name and URL
    urlLabel->setText(" — " + displayUrl);
    urlLabel->setStyleSheet("color: #aaaaaa;");
    urlLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // Add to layout
    itemLayout->addWidget(iconLabel);
    itemLayout->addWidget(nameLabel);
    itemLayout->addWidget(urlLabel, 1);

    // Add a Switch to Tab button if needed (could be implemented later)
    if (website.name == "Discord" || website.name == "GitHub") {
        auto switchButton = new QPushButton("Switch to Tab");
        switchButton->setStyleSheet(R"(
            QPushButton {
                background-color: #36383e;
                color: #aaaaaa;
                border: none;
                border-radius: 4px;
                padding: 5px 10px;
            }
            QPushButton:hover {
                background-color: #454750;
            }
        )");
        itemLayout->addWidget(switchButton);
    }

    // Set fixed height for the widget
    itemWidget->setFixedHeight(40);

    // Set the custom widget
    item->setSizeHint(QSize(websiteList->width() - 20, 40));
    websiteList->setItemWidget(item, itemWidget);

    // Store the website URL as item data
    item->setData(Qt::UserRole, website.url);
}

void CommandPalette::updateListFromFilter(const QString &filter) {
    websiteList->clear();

    QString lowercaseFilter = filter.toLower();

    // If the filter is empty, add all websites
    if (filter.isEmpty()) {
        for (const auto &website : websites) {
            addWebsiteToList(website);
        }
        return;
    }

    // Check if the filter matches any website
    for (const auto &website : websites) {
        if (website.name.toLower().contains(lowercaseFilter) ||
            website.url.toLower().contains(lowercaseFilter)) {
            addWebsiteToList(website);
        }
    }

    // Add a custom URL option if it looks like a URL
    if (filter.contains(".") || filter.contains("://")) {
        Website customSite;
        customSite.name = "Go to";
        customSite.url = Utils::normalizeUrl(filter);
        addWebsiteToList(customSite);
    } else if (!filter.isEmpty()) {
        // Add search with Google option
        Website searchSite;
        searchSite.name = "Search Google for";
        searchSite.url = "https://www.google.com/search?q=" + filter;
        addWebsiteToList(searchSite);
    }

    // Select the first item if available
    if (websiteList->count() > 0) {
        websiteList->setCurrentRow(0);
    }
}

void CommandPalette::filterWebsites(const QString &text) {
    updateListFromFilter(text);
}

void CommandPalette::selectWebsite(QListWidgetItem *item) {
    if (!item)
        return;

    QString urlString = item->data(Qt::UserRole).toString();
    QUrl url = createUrl(urlString);

    emit urlSelected(url);
    accept();
}

void CommandPalette::itemActivated(QListWidgetItem *item) {
    selectWebsite(item);
}

QUrl CommandPalette::getSelectedUrl() const {
    QListWidgetItem *item = websiteList->currentItem();
    if (!item) {
        return createUrl(searchBox->text());
    }

    return createUrl(item->data(Qt::UserRole).toString());
}

QUrl CommandPalette::createUrl(const QString &input) const {
    // If the input is empty, return Google
    if (input.isEmpty()) {
        return QUrl("https://www.google.com");
    }

    // If it looks like a URL, normalize it
    if (input.contains(".") || input.contains("://")) {
        return QUrl(Utils::normalizeUrl(input));
    }

    // Otherwise, use it as a search query
    return QUrl("https://www.google.com/search?q=" + input);
}

void CommandPalette::showEvent(QShowEvent *event) {
    QDialog::showEvent(event);

    // Center the palette on the screen
    QRect screenGeometry = QApplication::primaryScreen()->geometry();
    move((screenGeometry.width() - width()) / 2, (screenGeometry.height() - height()) / 3);

    // Clear and focus the search box
    searchBox->clear();
    searchBox->setFocus();

    // Populate the list with all websites
    updateListFromFilter("");
}

bool CommandPalette::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        auto keyEvent = static_cast<QKeyEvent *>(event);

        // Handle Enter/Return key
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            if (watched == searchBox) {
                // If the list has items and searchBox is focused, select the first item
                if (websiteList->count() > 0) {
                    selectWebsite(websiteList->item(0));
                    return true;
                } else {
                    // Create URL from text input
                    emit urlSelected(createUrl(searchBox->text()));
                    accept();
                    return true;
                }
            } else if (watched == websiteList) {
                // If list is focused, select the current item
                selectWebsite(websiteList->currentItem());
                return true;
            }
        }

        // Handle Tab key to navigate to list
        if (keyEvent->key() == Qt::Key_Tab && watched == searchBox) {
            if (websiteList->count() > 0) {
                websiteList->setFocus();
                websiteList->setCurrentRow(0);
                return true;
            }
        }

        // Handle Escape key to close
        if (keyEvent->key() == Qt::Key_Escape) {
            reject();
            return true;
        }

        // Handle up/down keys to navigate the list
        if (watched == searchBox) {
            if (keyEvent->key() == Qt::Key_Down && websiteList->count() > 0) {
                websiteList->setFocus();
                websiteList->setCurrentRow(0);
                return true;
            }
        }
    }

    // Pass unhandled events
    return QDialog::eventFilter(watched, event);
}
