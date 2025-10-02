#ifndef SQL_EDITOR_H
#define SQL_EDITOR_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTableView>
#include <QStandardItemModel>
#include <QLabel>
#include <QSplitter>
#include <QSyntaxHighlighter>
#include "../core/query_executor.h"

class SQLHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit SQLHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat keywordFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat commentFormat;
    QTextCharFormat numberFormat;
};

class SQLEditor : public QWidget {
    Q_OBJECT

public:
    explicit SQLEditor(QWidget *parent = nullptr);

    void setDatabaseContext(const QString &connectionName, const QString &database = QString(), const QString &schema = QString());

signals:
    void errorOccurred(const QString &error);

private slots:
    void executeQuery();
    void displayQueryResult(const QueryResult &result);

private:
    void setupUI();

    QComboBox *contextCombo;
    QPlainTextEdit *editor;
    QPushButton *executeButton;
    QTableView *resultView;
    QStandardItemModel *resultModel;
    QLabel *statusLabel;
    SQLHighlighter *highlighter;
    QueryExecutor *queryExecutor;

    QString currentConnectionName;
    QString currentDatabase;
    QString currentSchema;
};

#endif // SQL_EDITOR_H
