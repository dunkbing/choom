#include "ui/connection_dialog.h"
#include "database/connection_manager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>

ConnectionDialog::ConnectionDialog(QWidget *parent)
    : QDialog(parent) {
    setupUI();
    setWindowTitle("New Database Connection");
    resize(500, 400);
}

void ConnectionDialog::setupUI() {
    auto *mainLayout = new QVBoxLayout(this);

    // Connection name
    auto *nameLayout = new QHBoxLayout();
    nameLayout->addWidget(new QLabel("Connection Name:", this));
    nameEdit = new QLineEdit(this);
    nameEdit->setText("My SQLite Database");
    nameLayout->addWidget(nameEdit);
    mainLayout->addLayout(nameLayout);

    // Database type selector
    auto *typeLayout = new QHBoxLayout();
    typeLayout->addWidget(new QLabel("Database Type:", this));
    typeCombo = new QComboBox(this);
    typeCombo->addItem("SQLite");
    typeCombo->addItem("MySQL");
    typeCombo->addItem("PostgreSQL");
    connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ConnectionDialog::onDatabaseTypeChanged);
    typeLayout->addWidget(typeCombo);
    mainLayout->addLayout(typeLayout);

    // Stacked widget for different database forms
    formStack = new QStackedWidget(this);

    // SQLite form
    auto *sqliteWidget = new QWidget();
    setupSQLiteForm(sqliteWidget);
    formStack->addWidget(sqliteWidget);

    // MySQL form
    auto *mysqlWidget = new QWidget();
    setupMySQLForm(mysqlWidget);
    formStack->addWidget(mysqlWidget);

    // PostgreSQL form
    auto *postgresWidget = new QWidget();
    setupPostgreSQLForm(postgresWidget);
    formStack->addWidget(postgresWidget);

    mainLayout->addWidget(formStack, 1);

    // Buttons
    auto *buttonLayout = new QHBoxLayout();

    testButton = new QPushButton("Test Connection", this);
    connect(testButton, &QPushButton::clicked, this, &ConnectionDialog::testConnection);
    buttonLayout->addWidget(testButton);

    buttonLayout->addStretch();

    cancelButton = new QPushButton("Cancel", this);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(cancelButton);

    okButton = new QPushButton("Connect", this);
    okButton->setDefault(true);
    connect(okButton, &QPushButton::clicked, this, &ConnectionDialog::onAccept);
    buttonLayout->addWidget(okButton);

    mainLayout->addLayout(buttonLayout);
}

void ConnectionDialog::setupSQLiteForm(QWidget *widget) {
    auto *layout = new QFormLayout(widget);

    sqlitePathEdit = new QLineEdit(widget);
    sqlitePathEdit->setPlaceholderText("/path/to/database.sqlite");

    browseSQLiteButton = new QPushButton("Browse...", widget);
    connect(browseSQLiteButton, &QPushButton::clicked, this, &ConnectionDialog::browseForSQLiteFile);

    auto *pathLayout = new QHBoxLayout();
    pathLayout->addWidget(sqlitePathEdit);
    pathLayout->addWidget(browseSQLiteButton);

    layout->addRow("Database File:", pathLayout);

    auto *infoLabel = new QLabel(
        "Select an existing SQLite database file or create a new one.", widget);
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: #888; font-size: 11px;");
    layout->addRow(infoLabel);
}

void ConnectionDialog::setupMySQLForm(QWidget *widget) {
    auto *layout = new QFormLayout(widget);

    mysqlHostEdit = new QLineEdit(widget);
    mysqlHostEdit->setText("localhost");
    layout->addRow("Host:", mysqlHostEdit);

    mysqlPortSpin = new QSpinBox(widget);
    mysqlPortSpin->setRange(1, 65535);
    mysqlPortSpin->setValue(3306);
    layout->addRow("Port:", mysqlPortSpin);

    mysqlDatabaseEdit = new QLineEdit(widget);
    mysqlDatabaseEdit->setPlaceholderText("database_name");
    layout->addRow("Database:", mysqlDatabaseEdit);

    mysqlUsernameEdit = new QLineEdit(widget);
    mysqlUsernameEdit->setPlaceholderText("root");
    layout->addRow("Username:", mysqlUsernameEdit);

    mysqlPasswordEdit = new QLineEdit(widget);
    mysqlPasswordEdit->setEchoMode(QLineEdit::Password);
    mysqlPasswordEdit->setPlaceholderText("password");
    layout->addRow("Password:", mysqlPasswordEdit);
}

void ConnectionDialog::setupPostgreSQLForm(QWidget *widget) {
    auto *layout = new QFormLayout(widget);

    postgresHostEdit = new QLineEdit(widget);
    postgresHostEdit->setText("localhost");
    layout->addRow("Host:", postgresHostEdit);

    postgresPortSpin = new QSpinBox(widget);
    postgresPortSpin->setRange(1, 65535);
    postgresPortSpin->setValue(5432);
    layout->addRow("Port:", postgresPortSpin);

    postgresDatabaseEdit = new QLineEdit(widget);
    postgresDatabaseEdit->setPlaceholderText("postgres");
    layout->addRow("Database:", postgresDatabaseEdit);

    postgresUsernameEdit = new QLineEdit(widget);
    postgresUsernameEdit->setPlaceholderText("postgres");
    layout->addRow("Username:", postgresUsernameEdit);

    postgresPasswordEdit = new QLineEdit(widget);
    postgresPasswordEdit->setEchoMode(QLineEdit::Password);
    postgresPasswordEdit->setPlaceholderText("password");
    layout->addRow("Password:", postgresPasswordEdit);
}

void ConnectionDialog::onDatabaseTypeChanged(int index) {
    formStack->setCurrentIndex(index);

    // Set default connection name based on database type
    switch (index) {
        case 0: // SQLite
            nameEdit->setText("My SQLite Database");
            break;
        case 1: // MySQL
            nameEdit->setText("My MySQL Connection");
            break;
        case 2: // PostgreSQL
            nameEdit->setText("My PostgreSQL Connection");
            break;
    }
}

void ConnectionDialog::browseForSQLiteFile() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Select SQLite Database",
        QDir::homePath(),
        "SQLite Database (*.sqlite *.sqlite3 *.db *.db3);;All Files (*)"
    );

    if (!fileName.isEmpty()) {
        sqlitePathEdit->setText(fileName);

        // Set connection name from filename
        QFileInfo fileInfo(fileName);
        nameEdit->setText(fileInfo.baseName());
    }
}

void ConnectionDialog::testConnection() {
    // Create a temporary connection config
    ConnectionConfig testConfig;
    testConfig.name = "Test Connection";

    int typeIndex = typeCombo->currentIndex();

    switch (typeIndex) {
        case 0: // SQLite
            testConfig.type = DatabaseType::SQLite;
            testConfig.filePath = sqlitePathEdit->text();
            if (testConfig.filePath.isEmpty()) {
                QMessageBox::warning(this, "Invalid Input", "Please specify a database file path.");
                return;
            }
            break;

        case 1: // MySQL
            testConfig.type = DatabaseType::MySQL;
            testConfig.host = mysqlHostEdit->text();
            testConfig.port = mysqlPortSpin->value();
            testConfig.database = mysqlDatabaseEdit->text();
            testConfig.username = mysqlUsernameEdit->text();
            testConfig.password = mysqlPasswordEdit->text();
            break;

        case 2: // PostgreSQL
            testConfig.type = DatabaseType::PostgreSQL;
            testConfig.host = postgresHostEdit->text();
            testConfig.port = postgresPortSpin->value();
            testConfig.database = postgresDatabaseEdit->text();
            testConfig.username = postgresUsernameEdit->text();
            testConfig.password = postgresPasswordEdit->text();
            break;
    }

    // Try to connect
    auto connection = ConnectionManager::instance().createConnection(testConfig);
    if (connection) {
        if (connection->connect()) {
            QMessageBox::information(this, "Success", "Connection test successful!");
            connection->disconnect();
        } else {
            QMessageBox::critical(this, "Connection Failed",
                "Failed to connect:\n" + connection->getLastError());
        }
    }
}

void ConnectionDialog::onAccept() {
    // Validate connection name
    if (nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Please enter a connection name.");
        return;
    }

    config.name = nameEdit->text().trimmed();

    int typeIndex = typeCombo->currentIndex();

    switch (typeIndex) {
        case 0: // SQLite
            config.type = DatabaseType::SQLite;
            config.filePath = sqlitePathEdit->text();
            if (config.filePath.isEmpty()) {
                QMessageBox::warning(this, "Invalid Input", "Please specify a database file path.");
                return;
            }
            break;

        case 1: // MySQL
            config.type = DatabaseType::MySQL;
            config.host = mysqlHostEdit->text();
            config.port = mysqlPortSpin->value();
            config.database = mysqlDatabaseEdit->text();
            config.username = mysqlUsernameEdit->text();
            config.password = mysqlPasswordEdit->text();

            if (config.host.isEmpty() || config.username.isEmpty()) {
                QMessageBox::warning(this, "Invalid Input", "Please fill in host and username.");
                return;
            }
            break;

        case 2: // PostgreSQL
            config.type = DatabaseType::PostgreSQL;
            config.host = postgresHostEdit->text();
            config.port = postgresPortSpin->value();
            config.database = postgresDatabaseEdit->text();
            config.username = postgresUsernameEdit->text();
            config.password = postgresPasswordEdit->text();

            if (config.host.isEmpty() || config.username.isEmpty()) {
                QMessageBox::warning(this, "Invalid Input", "Please fill in host and username.");
                return;
            }
            break;
    }

    accept();
}

ConnectionConfig ConnectionDialog::getConnectionConfig() const {
    return config;
}
