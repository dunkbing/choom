#ifndef CONNECTION_DIALOG_H
#define CONNECTION_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QStackedWidget>
#include <QFormLayout>
#include "database/database_connection.h"

class ConnectionDialog : public QDialog {
    Q_OBJECT

public:
    explicit ConnectionDialog(QWidget *parent = nullptr);

    ConnectionConfig getConnectionConfig() const;

private slots:
    void onDatabaseTypeChanged(int index);
    void browseForSQLiteFile();
    void testConnection();
    void onAccept();

private:
    void setupUI();
    void setupSQLiteForm(QWidget *widget);
    void setupMySQLForm(QWidget *widget);
    void setupPostgreSQLForm(QWidget *widget);

    // Common fields
    QLineEdit *nameEdit;
    QComboBox *typeCombo;
    QStackedWidget *formStack;

    // SQLite fields
    QLineEdit *sqlitePathEdit;
    QPushButton *browseSQLiteButton;

    // MySQL fields
    QLineEdit *mysqlHostEdit;
    QSpinBox *mysqlPortSpin;
    QLineEdit *mysqlDatabaseEdit;
    QLineEdit *mysqlUsernameEdit;
    QLineEdit *mysqlPasswordEdit;

    // PostgreSQL fields
    QLineEdit *postgresHostEdit;
    QSpinBox *postgresPortSpin;
    QLineEdit *postgresDatabaseEdit;
    QLineEdit *postgresUsernameEdit;
    QLineEdit *postgresPasswordEdit;

    QPushButton *testButton;
    QPushButton *okButton;
    QPushButton *cancelButton;

    ConnectionConfig config;
};

#endif // CONNECTION_DIALOG_H
