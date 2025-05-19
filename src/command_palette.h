//
// Created by Bùi Đặng Bình on 18/5/25.
//

#ifndef COMMAND_PALETTE_H
#define COMMAND_PALETTE_H

#include <QDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QMap>
#include <QUrl>
#include <QVBoxLayout>

class Website {
public:
    QString name;
    QString url;
    QString icon;

    Website(const QString &name, const QString &url, const QString &icon)
        : name(name), url(url), icon(icon) {}

    Website() : name(""), url(""), icon("") {}
};

class CommandPalette : public QDialog {
    Q_OBJECT

public:
    explicit CommandPalette(QWidget *parent = nullptr);
    ~CommandPalette() override;

    QUrl getSelectedUrl() const;
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void urlSelected(const QUrl &url);

private slots:
    void filterWebsites(const QString &text);
    void selectWebsite(QListWidgetItem *item);
    void itemActivated(QListWidgetItem *item);

private:
    QLineEdit *searchBox;
    QListWidget *websiteList;
    QVBoxLayout *layout;

    QMap<QString, Website> websites;

    void initializeUI();
    void initializeWebsites();
    void addWebsiteToList(const Website &website) const;
    void updateListFromFilter(const QString &filter);
    QUrl createUrl(const QString &input) const;
};

#endif // COMMAND_PALETTE_H
