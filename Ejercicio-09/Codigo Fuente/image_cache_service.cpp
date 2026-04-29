#include "image_cache_service.h"
#include "logger.h"

#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDebug>

ImageCacheService::ImageCacheService(QObject *parent)
    : QObject(parent)
{
}

QString ImageCacheService::cacheDir() const
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return base + "/cache";
}

QString ImageCacheService::safeKeyToFilename(const QString &cacheKey) const
{
    QString s = cacheKey;
    s.replace("/", "_").replace("\\", "_").replace(":", "_");
    if (s.trimmed().isEmpty()) s = "image";
    return s + ".img";
}

QString ImageCacheService::cacheFilePath(const QString &cacheKey) const
{
    QDir().mkpath(cacheDir());
    return cacheDir() + "/" + safeKeyToFilename(cacheKey);
}

void ImageCacheService::fetchImage(const QUrl &url, const QString &cacheKey)
{
    if (!url.isValid()) {
        Logger::log("IMG", "URL inválida.");
        emit imageError("URL inválida.");
        return;
    }

    const QString path = cacheFilePath(cacheKey);

    // 1) Cache local
    if (QFile::exists(path)) {
        QPixmap pm;
        if (pm.load(path)) {
            Logger::log("IMG", "Cache HIT: " + path);
            emit imageReady(pm, path);
            return;
        } else {
            Logger::log("IMG", "Cache corrupto, borrando: " + path);
            QFile::remove(path);
        }
    }

    // 2) Descarga
    Logger::log("IMG", "Downloading: " + url.toString());
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, "QtImageCache/1.0");

    QNetworkReply *reply = m_nam.get(req);

    connect(reply, &QNetworkReply::finished, this, [this, reply, path]() {
        const int http = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const auto err = reply->error();
        const QString errStr = reply->errorString();
        const QByteArray bytes = reply->readAll();
        reply->deleteLater();

        qDebug() << "[Image] HTTP status:" << http;
        qDebug() << "[Image] Network error:" << err << errStr;
        qDebug() << "[Image] Bytes:" << bytes.size();

        if (err != QNetworkReply::NoError) {
            Logger::log("IMG", "ERROR red: " + errStr);
            emit imageError(QString("Error de red: %1").arg(errStr));
            return;
        }
        if (bytes.isEmpty()) {
            Logger::log("IMG", "ERROR: imagen vacía.");
            emit imageError("Imagen vacía.");
            return;
        }

        // Guardar en cache
        QDir().mkpath(QFileInfo(path).absolutePath());
        QFile f(path);
        if (!f.open(QIODevice::WriteOnly)) {
            Logger::log("IMG", "ERROR: no se pudo escribir cache: " + path);
            emit imageError("No se pudo escribir la imagen en cache.");
            return;
        }
        f.write(bytes);
        f.close();

        QPixmap pm;
        if (!pm.load(path)) {
            QFile::remove(path);
            Logger::log("IMG", "ERROR: imagen descargada inválida (no se pudo cargar).");
            emit imageError("La imagen descargada no es válida.");
            return;
        }

        Logger::log("IMG", "OK: imagen lista en cache: " + path);
        emit imageReady(pm, path);
    });
}