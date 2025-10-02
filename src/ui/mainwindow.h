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
#include "table_viewer.h"
#include "sql_editor.h"
#include "welcome_widget.h"
#include "connection_dialog.h"
#include "../database/connection_manager.h"

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
    void openTableInTab(const QString &connectionName, const QString &tableName);
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
