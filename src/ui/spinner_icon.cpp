#include "spinner_icon.h"
#include <QQmlEngine>
#include <QQmlComponent>
#include <QPainter>
#include <QPixmap>
#include <QImage>

SpinnerIcon::SpinnerIcon(QObject *parent)
    : QObject(parent), rotation(0) {

    timer = new QTimer(this);
    timer->setInterval(100);  // Update every 100ms
    connect(timer, &QTimer::timeout, this, &SpinnerIcon::updateIcon);

    // Create QML spinner
    quickView = new QQuickView();
    quickView->setSource(QUrl("qrc:/qml/src/ui/Spinner.qml"));
    quickView->setColor(Qt::transparent);
    quickView->setResizeMode(QQuickView::SizeRootObjectToView);
    quickView->resize(16, 16);

    spinnerItem = quickView->rootObject();
}

SpinnerIcon::~SpinnerIcon() {
    delete quickView;
}

void SpinnerIcon::updateIcon() {
    if (!spinnerItem) {
        return;
    }

    // Grab the QML view as an image
    QImage image = quickView->grabWindow();

    QPixmap pixmap = QPixmap::fromImage(image);
    emit iconUpdated(QIcon(pixmap));
}

QIcon SpinnerIcon::getIcon() {
    if (!spinnerItem) {
        return QIcon();
    }

    // Grab the QML view as an image
    QImage image = quickView->grabWindow();

    return QIcon(QPixmap::fromImage(image));
}

void SpinnerIcon::start() {
    if (!timer->isActive()) {
        timer->start();
    }
}

void SpinnerIcon::stop() {
    timer->stop();
}
