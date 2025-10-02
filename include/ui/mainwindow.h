#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeView>
#include <QTabWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <QSplitter>

#include "title_bar.h"
#include "connection_tree_model.h"
#include "welcome_widget.h"

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void toggleSidebar();

private slots:
    void addNewConnection();
    void onTreeItemDoubleClicked(const QModelIndex &index);
    void onTreeItemContextMenu(const QPoint &pos);
    void openSQLEditor(const QString &connectionName = QString(), const QString &database = QString(), const QString &schema = QString());

private:
    void setupUI();
    void setupSidebar();
    void loadSavedConnections();
    void openTableInTab(const QString &connectionName, const QString &tableName,
                        const QString &databaseName = QString(), const QString &schemaName = QString());
    int findTab(const QString &tabName);

    QSplitter *splitter;
    QWidget *sidebarWidget;
    QVBoxLayout *sidebarLayout;
    QTreeView *connectionTree;
    ConnectionTreeModel *treeModel;
    QTabWidget *tabWidget;
    TitleBar *titleBar;
    WelcomeWidget *welcomeWidget;
};

#endif // MAINWINDOW_H
