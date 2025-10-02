#include "table_viewer.h"
#include <QHeaderView>
#include <QHBoxLayout>
#include <QFutureWatcher>
#include <QStackedLayout>

TableViewer::TableViewer(QWidget *parent)
    : QWidget(parent), currentPage(0), pageSize(1000), totalRows(0) {
    setupUI();
    queryExecutor = new QueryExecutor(this);
}

void TableViewer::setupUI() {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(5);

    // Info bar
    auto *infoBar = new QWidget(this);
    auto *infoLayout = new QHBoxLayout(infoBar);
    infoLayout->setContentsMargins(10, 5, 10, 5);

    infoLabel = new QLabel("No data loaded", this);
    executionTimeLabel = new QLabel("", this);

    infoLayout->addWidget(infoLabel);
    infoLayout->addStretch();
    infoLayout->addWidget(executionTimeLabel);

    mainLayout->addWidget(infoBar);

    // Create stacked widget to switch between table and loading spinner
    auto *stackedWidget = new QWidget(this);
    auto *stackedLayout = new QStackedLayout(stackedWidget);
    stackedLayout->setContentsMargins(0, 0, 0, 0);

    // Table view
    tableView = new QTableView(this);
    tableModel = new QStandardItemModel(this);
    tableView->setModel(tableModel);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->setAlternatingRowColors(true);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    stackedLayout->addWidget(tableView);

    // Loading spinner (QML)
    loadingSpinner = new QQuickWidget(this);
    loadingSpinner->setSource(QUrl("qrc:/qml/src/ui/Spinner.qml"));
    loadingSpinner->setResizeMode(QQuickWidget::SizeRootObjectToView);
    loadingSpinner->setAttribute(Qt::WA_AlwaysStackOnTop);
    loadingSpinner->setClearColor(QColor("#1e1e1e"));
    stackedLayout->addWidget(loadingSpinner);

    mainLayout->addWidget(stackedWidget, 1);

    // Pagination controls
    auto *paginationBar = new QWidget(this);
    auto *paginationLayout = new QHBoxLayout(paginationBar);
    paginationLayout->setContentsMargins(10, 5, 10, 5);

    prevButton = new QPushButton("Previous", this);
    nextButton = new QPushButton("Next", this);
    pageSpinBox = new QSpinBox(this);
    pageSpinBox->setMinimum(1);
    pageLabel = new QLabel("Page:", this);

    connect(prevButton, &QPushButton::clicked, this, &TableViewer::previousPage);
    connect(nextButton, &QPushButton::clicked, this, &TableViewer::nextPage);
    connect(pageSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &TableViewer::goToPage);

    paginationLayout->addWidget(prevButton);
    paginationLayout->addWidget(nextButton);
    paginationLayout->addStretch();
    paginationLayout->addWidget(pageLabel);
    paginationLayout->addWidget(pageSpinBox);

    mainLayout->addWidget(paginationBar);
}

void TableViewer::loadTableData(const QString &connectionName, const QString &tableName,
                                 const QString &databaseName, const QString &schemaName) {
    currentConnectionName = connectionName;
    currentTableName = tableName;
    currentPage = 0;

    showLoadingSpinner();

    QFuture<QueryResult> future = queryExecutor->executeTableQuery(
        connectionName, tableName, databaseName, schemaName, pageSize, currentPage * pageSize
    );

    auto *watcher = new QFutureWatcher<QueryResult>(this);
    connect(watcher, &QFutureWatcher<QueryResult>::finished, this, [this, watcher]() {
        hideLoadingSpinner();
        displayQueryResult(watcher->result());
        watcher->deleteLater();
    });
    watcher->setFuture(future);
}

void TableViewer::displayQueryResult(const QueryResult &result) {
    if (!result.success) {
        emit errorOccurred(result.errorMessage);
        return;
    }

    // Clear existing data
    tableModel->clear();

    // Set column headers
    tableModel->setHorizontalHeaderLabels(result.columnNames);

    // Add data rows
    for (const QSqlRecord &record : result.records) {
        QList<QStandardItem*> row;
        for (int i = 0; i < record.count(); ++i) {
            auto *item = new QStandardItem(record.value(i).toString());
            row.append(item);
        }
        tableModel->appendRow(row);
    }

    // Update info labels
    totalRows = result.rowCount;
    infoLabel->setText(QString("%1 rows").arg(result.rowCount));
    executionTimeLabel->setText(QString("Execution time: %1 ms").arg(result.executionTimeMs));

    updatePaginationInfo();
}

void TableViewer::nextPage() {
    currentPage++;
    loadTableData(currentConnectionName, currentTableName);
}

void TableViewer::previousPage() {
    if (currentPage > 0) {
        currentPage--;
        loadTableData(currentConnectionName, currentTableName);
    }
}

void TableViewer::goToPage() {
    int page = pageSpinBox->value() - 1; // 0-indexed
    if (page >= 0) {
        currentPage = page;
        loadTableData(currentConnectionName, currentTableName);
    }
}

void TableViewer::updatePaginationInfo() {
    prevButton->setEnabled(currentPage > 0);
    pageSpinBox->setValue(currentPage + 1);

    // Enable next button if we got full page of results
    nextButton->setEnabled(totalRows >= pageSize);
}

void TableViewer::showLoadingSpinner() {
    loadingSpinner->show();
    loadingSpinner->raise();
    tableView->hide();
}

void TableViewer::hideLoadingSpinner() {
    loadingSpinner->hide();
    tableView->show();
}
