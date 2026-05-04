#pragma once
#include <QObject>
#include <QtWebSockets/QWebSocket>
#include <QString>
#include <QJsonObject>

/**
 * WsClient wraps a QWebSocket connection to the Kanban backend.
 * Authentication via query string: /ws?u=USER&p=PASS
 * Emits wsEvent(type, data) for each incoming message.
 */
class WsClient : public QObject
{
    Q_OBJECT
public:
    explicit WsClient(QObject *parent = nullptr);

    void connectToServer(const QString &wsBaseUrl, const QString &user, const QString &pass);
    void disconnect();
    bool isConnected() const;

signals:
    void wsEvent(const QString &type, const QJsonObject &data);
    void connected();
    void disconnected();
    void wsError(const QString &message);

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void onError(QAbstractSocket::SocketError error);

private:
    QWebSocket *m_socket;
};
