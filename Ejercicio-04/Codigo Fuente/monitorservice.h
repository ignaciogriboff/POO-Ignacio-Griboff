#pragma once

#include <QObject>
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointer>
#include <QTimer>
#include <QUrl>

struct VpsMetrics {
    bool ok = false;
    QString error;

    QDateTime timestampUtc;
    qint64 uptimeSeconds = 0;

    double load1 = 0, load5 = 0, load15 = 0;

    int memTotalMb = 0;
    int memUsedMb = 0;
    double memUsedPercent = 0;

    QString diskMount;
    double diskTotalGb = 0;
    double diskUsedGb = 0;
    double diskUsedPercent = 0;
};

class MonitorService : public QObject {
    Q_OBJECT
public:
    explicit MonitorService(QObject *parent = nullptr);

    void setEndpointUrl(const QUrl &url);
    QUrl endpointUrl() const;

    void setIntervalSeconds(int seconds);
    int intervalSeconds() const;

    void setRequestTimeoutMs(int ms);
    int requestTimeoutMs() const;

    void start();
    void stop();

public slots:
    void refreshNow();

signals:
    void metricsUpdated(const VpsMetrics &m);
    void requestStateChanged(bool inFlight);

private slots:
    void onReplyFinished();
    void onRequestTimeout();

private:
    VpsMetrics parseJson(const QByteArray &data);

    QUrl m_url;
    QNetworkAccessManager m_nam;
    QPointer<QNetworkReply> m_reply;

    QTimer m_timer;
    int m_intervalSeconds = 10;

    QTimer m_timeoutTimer;
    int m_timeoutMs = 3000;
};