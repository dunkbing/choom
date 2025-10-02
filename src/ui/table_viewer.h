#ifndef TABLE_VIEWER_H
#define TABLE_VIEWER_H

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include "../core/query_executor.h"

class TableViewer : public QWidget {
    Q_OBJECT

public:
    explicit TableViewer(QWidget *parent = nullptr);

    void loadTableData(const QString &connectionName, const QString &tableName);
    void displayQueryResult(const QueryResult &result);

signals:
    void errorOccurred(const QString &error);

private slots:
    void nextPage();
    void previousPage();
    void goToPage();

private:
    void setupUI();
    void updatePaginationInfo();

    QTableView *tableView;
    QStandardItemModel *tableModel;
    QLabel *infoLabel;
    QLabel *executionTimeLabel;
    QPushButton *prevButton;
    QPushButton *nextButton;
    QSpinBox *pageSpinBox;
    QLabel *pageLabel;

    QueryExecutor *queryExecutor;
    QString currentConnectionName;
    QString currentTableName;
    int currentPage;
    int pageSize;
    int totalRows;
};

#endif // TABLE_VIEWER_H
