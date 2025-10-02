#ifndef WELCOME_WIDGET_H
#define WELCOME_WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

class WelcomeWidget : public QWidget {
    Q_OBJECT

public:
    explicit WelcomeWidget(QWidget *parent = nullptr);

private:
    void setupUI();
};

#endif // WELCOME_WIDGET_H
