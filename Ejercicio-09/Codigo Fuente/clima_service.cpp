#include "clima_service.h"

#include <QUrl>
#include <QUrlQuery>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QAuthenticator>
#include <QNetworkProxy>
#include <QDebug>
#include <limits>
#include <cmath>

static QString maskApiKey(const QString &key)
{
    if (key.size() <= 6) return "***";
    return key.left(3) + "..." + key.right(3);
}

ClimaService::ClimaService(QObject *parent)
    : QObject(parent)
{
    connect(&m_nam, &QNetworkAccessManager::proxyAuthenticationRequired,
            this, [](const QNetworkProxy &proxy, QAuthenticator *auth) {
                qDebug() << "[Proxy] Autenticación requerida para:"
                         << proxy.hostName() << proxy.port();
                Q_UNUSED(auth);
            });
}

void ClimaService::fetchClimaActual(const QString &city,
                                    const QString &apiKey,
                                    const QString &units)
{
    const QString k = apiKey.trimmed();
    if (k.isEmpty()) {
        emit climaError("Falta API key (configuración).");
        return;
    }
    if (city.trimmed().isEmpty()) {
        emit climaError("Falta ciudad (configuración).");
        return;
    }

    QUrl url("https://api.openweathermap.org/data/2.5/weather");
    QUrlQuery q;
    q.addQueryItem("q", city);       // "Cordoba,AR"
    q.addQueryItem("appid", k);
    q.addQueryItem("units", units);  // metric -> °C
    q.addQueryItem("lang", "es");
    url.setQuery(q);

    qDebug() << "[Clima] Request city:" << city
             << "units:" << units
             << "apiKey:" << maskApiKey(k);
    qDebug() << "[Clima] URL:" << url.toString(QUrl::FullyEncoded);

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, "QtLoginClima/1.0");

    QNetworkReply *reply = m_nam.get(req);

    connect(reply, &QNetworkReply::finished, this, [this, reply, city]() {
        const QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        const int http = statusCode.isValid() ? statusCode.toInt() : 0;

        const QByteArray body = reply->readAll();
        const auto netErr = reply->error();
        const QString netErrStr = reply->errorString();

        reply->deleteLater();

        qDebug() << "[Clima] HTTP status:" << http;
        qDebug() << "[Clima] Network error:" << netErr << netErrStr;

        // Si hay status HTTP 401, intentamos leer mensaje del servidor
        auto serverMessage = [&]() -> QString {
            QJsonParseError jerr{};
            const QJsonDocument d = QJsonDocument::fromJson(body, &jerr);
            if (jerr.error == QJsonParseError::NoError && d.isObject()) {
                const QJsonObject o = d.object();
                const QString msg = o.value("message").toString();
                if (!msg.isEmpty()) return msg;
            }
            return QString();
        };

        if (netErr != QNetworkReply::NoError) {
            if (http == 401) {
                const QString msg = serverMessage();
                if (!msg.isEmpty()) {
                    emit climaError(QString("No autorizado (401). Revisá tu API key. Servidor: %1").arg(msg));
                } else {
                    emit climaError("No autorizado (401). Revisá tu API key.");
                }
                return;
            }

            emit climaError(QString("Error de red: %1").arg(netErrStr));
            return;
        }

        // A veces el netErr es NoError pero el HTTP puede ser != 200 igual
        if (http != 200) {
            const QString msg = serverMessage();
            emit climaError(QString("HTTP %1. %2").arg(http).arg(msg.isEmpty() ? "Error al consultar clima." : msg));
            return;
        }

        // Parse OK
        QJsonParseError jerr{};
        const QJsonDocument doc = QJsonDocument::fromJson(body, &jerr);
        if (jerr.error != QJsonParseError::NoError || !doc.isObject()) {
            emit climaError("Respuesta inválida (JSON).");
            return;
        }

        const QJsonObject o = doc.object();

        if (!o.contains("main") || !o.value("main").isObject()) {
            emit climaError("Respuesta sin campo 'main'.");
            return;
        }

        const QJsonObject main = o.value("main").toObject();
        const double temp = main.value("temp").toDouble(std::numeric_limits<double>::quiet_NaN());
        if (std::isnan(temp)) {
            emit climaError("Respuesta sin temperatura.");
            return;
        }

        QString desc = "Sin descripción";
        if (o.contains("weather") && o.value("weather").isArray()) {
            const QJsonArray arr = o.value("weather").toArray();
            if (!arr.isEmpty() && arr.first().isObject()) {
                desc = arr.first().toObject().value("description").toString(desc);
            }
        }

        const int tz = o.value("timezone").toInt(0);
        const qint64 dt = static_cast<qint64>(o.value("dt").toDouble(0));

        ClimaActual c;
        c.ciudad = city;
        c.temperaturaC = temp;
        c.descripcion = desc;
        c.timezoneOffsetSec = tz;
        c.dtUtc = dt;

        emit climaOk(c);
    });
}