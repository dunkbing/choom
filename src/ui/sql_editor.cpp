#include "sql_editor.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QShortcut>
#include <QFutureWatcher>
#include <QHeaderView>

// SQL Syntax Highlighter
SQLHighlighter::SQLHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent) {

    // Keywords
    keywordFormat.setForeground(QColor(86, 156, 214)); // Blue
    keywordFormat.setFontWeight(QFont::Bold);

    QStringList keywordPatterns = {
        "\\bSELECT\\b", "\\bFROM\\b", "\\bWHERE\\b", "\\bJOIN\\b", "\\bINNER\\b",
        "\\bLEFT\\b", "\\bRIGHT\\b", "\\bOUTER\\b", "\\bON\\b", "\\bAS\\b",
        "\\bINSERT\\b", "\\bINTO\\b", "\\bVALUES\\b", "\\bUPDATE\\b", "\\bSET\\b",
        "\\bDELETE\\b", "\\bCREATE\\b", "\\bTABLE\\b", "\\bDROP\\b", "\\bALTER\\b",
        "\\bAND\\b", "\\bOR\\b", "\\bNOT\\b", "\\bNULL\\b", "\\bLIKE\\b",
        "\\bIN\\b", "\\bBETWEEN\\b", "\\bORDER\\b", "\\bBY\\b", "\\bGROUP\\b",
        "\\bHAVING\\b", "\\bLIMIT\\b", "\\bOFFSET\\b", "\\bDISTINCT\\b", "\\bCOUNT\\b",
        "\\bSUM\\b", "\\bAVG\\b", "\\bMAX\\b", "\\bMIN\\b", "\\bCASE\\b",
        "\\bWHEN\\b", "\\bTHEN\\b", "\\bELSE\\b", "\\bEND\\b"
    };

    for (const QString &pattern : keywordPatterns) {
        HighlightingRule rule;
        rule.pattern = QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // String literals
    stringFormat.setForeground(QColor(206, 145, 120)); // Orange
    HighlightingRule stringRule;
    stringRule.pattern = QRegularExpression("'[^']*'");
    stringRule.format = stringFormat;
    highlightingRules.append(stringRule);

    // Comments
    commentFormat.setForeground(QColor(106, 153, 85)); // Green
    commentFormat.setFontItalic(true);
    HighlightingRule commentRule;
    commentRule.pattern = QRegularExpression("--[^\n]*");
    commentRule.format = commentFormat;
    highlightingRules.append(commentRule);

    // Numbers
    numberFormat.setForeground(QColor(181, 206, 168)); // Light green
    HighlightingRule numberRule;
    numberRule.pattern = QRegularExpression("\\b[0-9]+\\b");
    numberRule.format = numberFormat;
    highlightingRules.append(numberRule);
}

void SQLHighlighter::highlightBlock(const QString &text) {
    for (const HighlightingRule &rule : highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}

// SQL Editor
SQLEditor::SQLEditor(QWidget *parent)
    : QWidget(parent) {
    setupUI();
    queryExecutor = new QueryExecutor(this);
}

void SQLEditor::setupUI() {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(5);

    // Top bar with context selector and execute button
    auto *topBar = new QWidget(this);
    auto *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(10, 5, 10, 5);

    auto *contextLabel = new QLabel("Context:", this);
    contextCombo = new QComboBox(this);
    contextCombo->setMinimumWidth(200);

    executeButton = new QPushButton("Execute", this);
    executeButton->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return));

    topLayout->addWidget(contextLabel);
    topLayout->addWidget(contextCombo);
    topLayout->addStretch();
    topLayout->addWidget(executeButton);

    mainLayout->addWidget(topBar);

    // Splitter for editor and results
    auto *splitter = new QSplitter(Qt::Vertical, this);

    // SQL Editor
    editor = new QPlainTextEdit(this);
    editor->setPlaceholderText("Enter SQL query here...");
    highlighter = new SQLHighlighter(editor->document());

    // Set editor style
    QFont font("Monaco");
    font.setPointSize(12);
    editor->setFont(font);

    splitter->addWidget(editor);

    // Results area
    auto *resultsWidget = new QWidget(this);
    auto *resultsLayout = new QVBoxLayout(resultsWidget);
    resultsLayout->setContentsMargins(0, 0, 0, 0);

    statusLabel = new QLabel("Ready", this);
    statusLabel->setStyleSheet("padding: 5px;");

    resultView = new QTableView(this);
    resultModel = new QStandardItemModel(this);
    resultView->setModel(resultModel);
    resultView->horizontalHeader()->setStretchLastSection(true);
    resultView->setAlternatingRowColors(true);
    resultView->setSelectionBehavior(QAbstractItemView::SelectRows);

    resultsLayout->addWidget(statusLabel);
    resultsLayout->addWidget(resultView);

    splitter->addWidget(resultsWidget);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);

    mainLayout->addWidget(splitter, 1);

    // Connect signals
    connect(executeButton, &QPushButton::clicked, this, &SQLEditor::executeQuery);
}

void SQLEditor::setDatabaseContext(const QString &connectionName, const QString &database, const QString &schema) {
    currentConnectionName = connectionName;
    currentDatabase = database;
    currentSchema = schema;

    // Update context combo
    QString contextText = connectionName;
    if (!database.isEmpty()) {
        contextText += " / " + database;
    }
    if (!schema.isEmpty()) {
        contextText += " / " + schema;
    }

    contextCombo->clear();
    contextCombo->addItem(contextText);
}

void SQLEditor::executeQuery() {
    QString query = editor->toPlainText().trimmed();
    if (query.isEmpty()) {
        return;
    }

    statusLabel->setText("Executing query...");
    executeButton->setEnabled(false);

    QFuture<QueryResult> future = queryExecutor->executeQuery(currentConnectionName, query);

    auto *watcher = new QFutureWatcher<QueryResult>(this);
    connect(watcher, &QFutureWatcher<QueryResult>::finished, this, [this, watcher]() {
        displayQueryResult(watcher->result());
        executeButton->setEnabled(true);
        watcher->deleteLater();
    });
    watcher->setFuture(future);
}

void SQLEditor::displayQueryResult(const QueryResult &result) {
    if (!result.success) {
        statusLabel->setText("Error: " + result.errorMessage);
        emit errorOccurred(result.errorMessage);
        resultModel->clear();
        return;
    }

    // Clear existing data
    resultModel->clear();

    // Set column headers
    resultModel->setHorizontalHeaderLabels(result.columnNames);

    // Add data rows
    for (const QSqlRecord &record : result.records) {
        QList<QStandardItem*> row;
        for (int i = 0; i < record.count(); ++i) {
            auto *item = new QStandardItem(record.value(i).toString());
            row.append(item);
        }
        resultModel->appendRow(row);
    }

    // Update status
    statusLabel->setText(
        QString("%1 rows returned | Execution time: %2 ms")
            .arg(result.rowCount)
            .arg(result.executionTimeMs)
    );
}
