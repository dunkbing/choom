//
// Created by Bùi Đặng Bình on 18/5/25.
//
#include "utils.h"

#include <QDebug>
#include <QFile>
#include <QPainter>
#include <QSvgRenderer>
#include <QUrl>

namespace Utils {
    QIcon createIconFromResource(const QString &resourcePath, const QColor &color) {
        if (resourcePath.endsWith(".png", Qt::CaseInsensitive)) {
            QPixmap pixmap(resourcePath);
            if (pixmap.isNull()) {
                qWarning() << "Could not load PNG resource:" << resourcePath;
                return QIcon();
            }

            // If color is not the default (white), apply a colorize effect
            if (color != Qt::white) {
                QImage image = pixmap.toImage();
                for (int y = 0; y < image.height(); ++y) {
                    for (int x = 0; x < image.width(); ++x) {
                        QColor pixelColor = QColor::fromRgba(image.pixel(x, y));
                        if (pixelColor.alpha() > 0) {
                            pixelColor.setRed(color.red());
                            pixelColor.setGreen(color.green());
                            pixelColor.setBlue(color.blue());
                            image.setPixelColor(x, y, pixelColor);
                        }
                    }
                }
                pixmap = QPixmap::fromImage(image);
            }

            return QIcon(pixmap);
        }

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

    QString normalizeUrl(const QString &url) {
        if (!url.isEmpty() && !url.contains("://")) {
            return "https://" + url;
        }
        return url;
    }

    QString createDisplayUrl(const QUrl &url) {
        QString displayUrl = url.host();
        if (displayUrl.isEmpty()) {
            displayUrl = url.toString();
        } else if (url.path().length() > 1) {
            displayUrl += url.path();
        }
        return displayUrl;
    }

    QString truncateString(const QString &str, int maxLength, const QString &ellipsis) {
        if (str.length() <= maxLength) {
            return str;
        }
        return str.left(maxLength - ellipsis.length()) + ellipsis;
    }
} // namespace Utils
