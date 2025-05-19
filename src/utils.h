//
// Created by Bùi Đặng Bình on 18/5/25.
//

#ifndef UTILS_H
#define UTILS_H

#include <QColor>
#include <QIcon>
#include <QString>
#include <QUrl>

namespace Utils {
    /**
     * Creates an icon from an SVG resource file
     * @param resourcePath Path to the SVG resource
     * @param color Color to apply to the SVG (default: white)
     * @return QIcon created from the SVG resource
     */
    QIcon createIconFromResource(const QString &resourcePath, const QColor &color = Qt::white);

    /**
     * Normalizes a URL by ensuring it has a scheme
     * @param url URL string to normalize
     * @return Normalized URL with scheme
     */
    QString normalizeUrl(const QString &url);

    /**
     * Creates a simplified display version of a URL
     * @param url URL to simplify
     * @return Simplified URL for display
     */
    QString createDisplayUrl(const QUrl &url);

    /**
     * Truncates a string if it exceeds maxLength
     * @param str String to truncate
     * @param maxLength Maximum length before truncation
     * @param ellipsis String to append when truncating (default: "...")
     * @return Truncated string, or original if short enough
     */
    QString truncateString(const QString &str, int maxLength, const QString &ellipsis = "...");
} // namespace Utils

#endif // UTILS_H
