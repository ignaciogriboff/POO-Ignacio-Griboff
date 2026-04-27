#include "realtimesyncservice.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

RealtimeSyncService::RealtimeSyncService(const QUrl& wsUrl, QObject* parent)
    : QObject(parent), m_wsUrl(wsUrl)
{
    connect(&m_ws, &QWebSocket::connected, this, &RealtimeSyncService::onConnected);
    connect(&m_ws, &QWebSocket::disconnected, this, &RealtimeSyncService::onDisconnected);
    connect(&m_ws, &QWebSocket::textMessageReceived, this, &RealtimeSyncService::onTextMessageReceived);
    connect(&m_ws, &QWebSocket::errorOccurred, this, &RealtimeSyncService::onErrorOccurred);
}

bool RealtimeSyncService::isConnected() const {
    return m_ws.state() == QAbstractSocket::ConnectedState;
}

void RealtimeSyncService::connectToServer() {
    emit connectionStateChanged("Conectando WS...");
    m_ws.open(m_wsUrl);
}

void RealtimeSyncService::disconnectFromServer() {
    m_ws.close();
}

void RealtimeSyncService::onConnected() {
    emit connectionStateChanged("WS conectado");
}

void RealtimeSyncService::onDisconnected() {
    emit connectionStateChanged("WS desconectado");
}

void RealtimeSyncService::onErrorOccurred(QAbstractSocket::SocketError) {
    emit connectionStateChanged("Error WS: " + m_ws.errorString());
}

QJsonObject RealtimeSyncService::strokeMetaToJson(const Stroke& s) {
    QJsonObject so;
    so["id"] = s.id;
    if (!s.author.isEmpty()) so["author"] = s.author;
    so["eraser"] = s.eraser;
    so["width"] = s.width;

    QJsonObject col;
    col["r"] = s.color.red();
    col["g"] = s.color.green();
    col["b"] = s.color.blue();
    so["color"] = col;

    so["points"] = QJsonArray{}; // begin sin points
    return so;
}

bool RealtimeSyncService::strokeMetaFromJson(const QJsonObject& so, Stroke* out, QString* error) {
    auto fail = [&](const QString& msg) {
        if (error) *error = msg;
        return false;
    };
    if (!out) return fail("out nulo");

    Stroke s;
    s.id = so.value("id").toString();
    s.author = so.value("author").toString();
    s.eraser = so.value("eraser").toBool(false);
    s.width = so.value("width").toDouble(3.0);

    if (!so.contains("color") || !so["color"].isObject())
        return fail("stroke sin color");
    QJsonObject col = so["color"].toObject();
    s.color = QColor(col.value("r").toInt(0), col.value("g").toInt(0), col.value("b").toInt(0));

    s.points.clear();
    *out = std::move(s);
    return true;
}

void RealtimeSyncService::sendStrokeBegin(const Stroke& stroke) {
    if (!isConnected()) return;

    QJsonObject msg;
    msg["type"] = "stroke_begin";
    msg["stroke"] = strokeMetaToJson(stroke);

    m_ws.sendTextMessage(QString::fromUtf8(QJsonDocument(msg).toJson(QJsonDocument::Compact)));
}

void RealtimeSyncService::sendStrokePoints(const QString& strokeId, const QVector<QPointF>& points) {
    if (!isConnected() || strokeId.isEmpty() || points.isEmpty()) return;

    QJsonArray pts;
    for (const auto& p : points) {
        QJsonObject po;
        po["x"] = p.x();
        po["y"] = p.y();
        pts.append(po);
    }

    QJsonObject msg;
    msg["type"] = "stroke_points";
    msg["id"] = strokeId;
    msg["points"] = pts;

    m_ws.sendTextMessage(QString::fromUtf8(QJsonDocument(msg).toJson(QJsonDocument::Compact)));
}

void RealtimeSyncService::sendStrokeEnd(const QString& strokeId) {
    if (!isConnected() || strokeId.isEmpty()) return;

    QJsonObject msg;
    msg["type"] = "stroke_end";
    msg["id"] = strokeId;

    m_ws.sendTextMessage(QString::fromUtf8(QJsonDocument(msg).toJson(QJsonDocument::Compact)));
}

void RealtimeSyncService::onTextMessageReceived(const QString& message) {
    QJsonParseError pe;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &pe);
    if (pe.error != QJsonParseError::NoError || !doc.isObject()) return;

    QJsonObject msg = doc.object();
    QString type = msg.value("type").toString();

    // Snapshot: seguimos usando HTTP para cargar inicial (simple y estable)
    if (type == "snapshot") return;

    if (type == "stroke_begin") {
        if (!msg.contains("stroke") || !msg["stroke"].isObject()) return;
        Stroke s;
        QString err;
        if (!strokeMetaFromJson(msg["stroke"].toObject(), &s, &err)) return;
        emit remoteStrokeBegin(s);
        return;
    }

    if (type == "stroke_points") {
        QString id = msg.value("id").toString();
        if (id.isEmpty() || !msg.contains("points") || !msg["points"].isArray()) return;

        QVector<QPointF> pts;
        QJsonArray arr = msg["points"].toArray();
        pts.reserve(arr.size());
        for (const auto& v : arr) {
            if (!v.isObject()) continue;
            QJsonObject po = v.toObject();
            pts.push_back(QPointF(po.value("x").toDouble(), po.value("y").toDouble()));
        }
        if (!pts.isEmpty())
            emit remoteStrokePoints(id, pts);
        return;
    }

    if (type == "stroke_end") {
        QString id = msg.value("id").toString();
        if (!id.isEmpty())
            emit remoteStrokeEnd(id);
        return;
    }
}