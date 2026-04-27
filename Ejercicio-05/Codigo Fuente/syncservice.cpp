#include "syncservice.h"
#include "drawingmodel.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>

SyncService::SyncService(const QUrl& baseUrl, QObject* parent)
    : QObject(parent), m_baseUrl(baseUrl)
{
}

void SyncService::loadCanvas(DrawingModel* model) {
    QUrl url = m_baseUrl;
    url.setPath("/canvas");

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, "QtCanvas/1.0");

    QNetworkReply* reply = m_nam.get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, model]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            emit loadFinished(false, reply->errorString());
            return;
        }

        QByteArray body = reply->readAll();
        QJsonParseError pe;
        QJsonDocument doc = QJsonDocument::fromJson(body, &pe);
        if (pe.error != QJsonParseError::NoError || !doc.isObject()) {
            emit loadFinished(false, "JSON inválido recibido desde el servidor.");
            return;
        }

        QString err;
        if (!model->fromJson(doc.object(), &err)) {
            emit loadFinished(false, "No se pudo cargar el modelo: " + err);
            return;
        }

        emit loadFinished(true, "Canvas cargado (HTTP).");
    });
}

void SyncService::saveCanvas(const DrawingModel* model) {
    QUrl url = m_baseUrl;
    url.setPath("/canvas");

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, "QtCanvas/1.0");
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonDocument doc(model->toJson());
    QByteArray payload = doc.toJson(QJsonDocument::Compact);

    QNetworkReply* reply = m_nam.post(req, payload);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            emit saveFinished(false, reply->errorString());
            return;
        }

        emit saveFinished(true, "Guardado OK (HTTP).");
    });
}