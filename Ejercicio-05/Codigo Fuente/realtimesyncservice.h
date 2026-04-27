#pragma once

#include <QObject>
#include <QUrl>
#include <QtWebSockets/QWebSocket>

#include "drawingmodel.h"

class RealtimeSyncService : public QObject {
    Q_OBJECT
public:
    explicit RealtimeSyncService(const QUrl& wsUrl, QObject* parent = nullptr);

    void connectToServer();
    void disconnectFromServer();
    bool isConnected() const;

    // Streaming API
    void sendStrokeBegin(const Stroke& stroke);
    void sendStrokePoints(const QString& strokeId, const QVector<QPointF>& points);
    void sendStrokeEnd(const QString& strokeId);

signals:
    void connectionStateChanged(QString state);

    // Remoto -> UI
    void remoteStrokeBegin(const Stroke& stroke);
    void remoteStrokePoints(const QString& strokeId, const QVector<QPointF>& points);
    void remoteStrokeEnd(const QString& strokeId);

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString& message);
    void onErrorOccurred(QAbstractSocket::SocketError);

private:
    static QJsonObject strokeMetaToJson(const Stroke& s); // sin points
    static bool strokeMetaFromJson(const QJsonObject& obj, Stroke* out, QString* error);

private:
    QUrl m_wsUrl;
    QWebSocket m_ws;
};