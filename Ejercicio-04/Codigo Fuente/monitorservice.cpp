#include "monitorservice.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>

MonitorService::MonitorService(QObject *parent) : QObject(parent) {
    m_timer.setSingleShot(false);
    connect(&m_timer, &QTimer::timeout, this, &MonitorService::refreshNow);

    m_timeoutTimer.setSingleShot(true);
    connect(&m_timeoutTimer, &QTimer::timeout, this, &MonitorService::onRequestTimeout);
}

void MonitorService::setEndpointUrl(const QUrl &url) { m_url = url; }
QUrl MonitorService::endpointUrl() const { return m_url; }

void MonitorService::setIntervalSeconds(int seconds) {
    m_intervalSeconds = qMax(1, seconds);
    if (m_timer.isActive()) {
        m_timer.start(m_intervalSeconds * 1000);
    }
}
int MonitorService::intervalSeconds() const { return m_intervalSeconds; }

void MonitorService::setRequestTimeoutMs(int ms) {
    m_timeoutMs = qMax(200, ms);
}
int MonitorService::requestTimeoutMs() const { return m_timeoutMs; }

void MonitorService::start() {
    if (!m_timer.isActive())
        m_timer.start(m_intervalSeconds * 1000);
    refreshNow();
}

void MonitorService::stop() {
    m_timer.stop();
}

void MonitorService::refreshNow() {
    if (!m_url.isValid()) {
        VpsMetrics m;
        m.ok = false;
        m.error = "Endpoint URL inválida";
        emit metricsUpdated(m);
        return;
    }
    if (m_reply) return; // request en curso

    QNetworkRequest req(m_url);
    req.setHeader(QNetworkRequest::UserAgentHeader, "VpsMonitorPanel/1.0");
    m_reply = m_nam.get(req);

    m_timeoutTimer.start(m_timeoutMs);

    emit requestStateChanged(true);
    connect(m_reply, &QNetworkReply::finished, this, &MonitorService::onReplyFinished);
}

void MonitorService::onRequestTimeout() {
    if (!m_reply) return;
    // aborta el request; luego se emitirá finished() y caerá en onReplyFinished()
    m_reply->abort();
}

void MonitorService::onReplyFinished() {
    m_timeoutTimer.stop();
    emit requestStateChanged(false);

    if (!m_reply) return;

    QNetworkReply *reply = m_reply.data();
    m_reply = nullptr;

    VpsMetrics m;

    if (reply->error() != QNetworkReply::NoError) {
        m.ok = false;
        m.error = reply->errorString();
        emit metricsUpdated(m);
        reply->deleteLater();
        return;
    }

    m = parseJson(reply->readAll());
    emit metricsUpdated(m);

    reply->deleteLater();
}

VpsMetrics MonitorService::parseJson(const QByteArray &data) {
    VpsMetrics m;

    QJsonParseError err{};
    const QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        m.ok = false;
        m.error = "JSON inválido";
        return m;
    }

    const QJsonObject root = doc.object();

    m.timestampUtc = QDateTime::fromString(root.value("timestamp_utc").toString(), Qt::ISODate);
    m.uptimeSeconds = (qint64)root.value("uptime_seconds").toDouble();

    const QJsonObject load = root.value("loadavg").toObject();
    m.load1 = load.value("1").toDouble();
    m.load5 = load.value("5").toDouble();
    m.load15 = load.value("15").toDouble();

    const QJsonObject mem = root.value("memory").toObject();
    m.memTotalMb = mem.value("total_mb").toInt();
    m.memUsedMb = mem.value("used_mb").toInt();
    m.memUsedPercent = mem.value("used_percent").toDouble();

    const QJsonObject disk = root.value("disk").toObject();
    m.diskMount = disk.value("mount").toString();
    m.diskTotalGb = disk.value("total_gb").toDouble();
    m.diskUsedGb = disk.value("used_gb").toDouble();
    m.diskUsedPercent = disk.value("used_percent").toDouble();

    m.ok = true;
    return m;
}