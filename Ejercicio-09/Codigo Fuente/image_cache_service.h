#ifndef IMAGE_CACHE_SERVICE_H
#define IMAGE_CACHE_SERVICE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QPixmap>
#include <QUrl>

class ImageCacheService : public QObject
{
    Q_OBJECT
public:
    explicit ImageCacheService(QObject *parent = nullptr);

    void fetchImage(const QUrl &url, const QString &cacheKey);
    QString cacheFilePath(const QString &cacheKey) const;

signals:
    void imageReady(const QPixmap &pixmap, const QString &cachePath);
    void imageError(const QString &message);

private:
    QNetworkAccessManager m_nam;

    QString cacheDir() const;
    QString safeKeyToFilename(const QString &cacheKey) const;
};

#endif // IMAGE_CACHE_SERVICE_H