//
// Created by Bùi Đặng Bình on 18/5/25.
//
#include "utils.h"

#include <QSvgRenderer>
#include <QPainter>
#include <QFile>
#include <QUrl>
#include <QDebug>

namespace Utils {
    QIcon createIconFromResource(const QString& resourcePath, const QColor& color) {
        QFile file(resourcePath);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "Could not open resource file:" << resourcePath;
            return QIcon();
        }

        QString svgData = file.readAll();
        file.close();

        QSvgRenderer renderer(svgData.toUtf8());
        QPixmap pixmap(24, 24);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setPen(color);
        renderer.render(&painter);
        return QIcon(pixmap);
    }

    QString normalizeUrl(const QString& url) {
        if (!url.isEmpty() && !url.contains("://")) {
            return "https://" + url;
        }
        return url;
    }

    QString createDisplayUrl(const QUrl& url) {
        QString displayUrl = url.host();
        if (displayUrl.isEmpty()) {
            displayUrl = url.toString();
        } else if (url.path().length() > 1) {
            displayUrl += url.path();
        }
        return displayUrl;
    }

    QString truncateString(const QString& str, int maxLength, const QString& ellipsis) {
        if (str.length() <= maxLength) {
            return str;
        }
        return str.left(maxLength - ellipsis.length()) + ellipsis;
    }
}
