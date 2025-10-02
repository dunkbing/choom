#ifndef SPINNER_ICON_H
#define SPINNER_ICON_H

#include <QObject>
#include <QIcon>
#include <QTimer>
#include <QQuickView>
#include <QQuickItem>

class SpinnerIcon : public QObject {
    Q_OBJECT

public:
    explicit SpinnerIcon(QObject *parent = nullptr);
    ~SpinnerIcon();

    QIcon getIcon();
    void start();
    void stop();
    bool isRunning() const { return timer->isActive(); }

signals:
    void iconUpdated(const QIcon &icon);

private:
    void updateIcon();

    QTimer *timer;
    QQuickView *quickView;
    QQuickItem *spinnerItem;
    int rotation;
};

#endif // SPINNER_ICON_H
