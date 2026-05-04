#include "ws_client.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

WsClient::WsClient(QObject *parent)
    : QObject(parent)
    , m_socket(new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this))
{
    connect(m_socket, &QWebSocket::connected, this, &WsClient::onConnected);
    connect(m_socket, &QWebSocket::disconnected, this, &WsClient::onDisconnected);
    connect(m_socket, &QWebSocket::textMessageReceived, this, &WsClient::onTextMessageReceived);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
            this, &WsClient::onError);
}

void WsClient::connectToServer(const QString &wsBaseUrl, const QString &user, const QString &pass)
{
    // Credentials passed as query parameters (server_wins on auth fail)
    QString url = QString("%1/ws?u=%2&p=%3")
                      .arg(wsBaseUrl)
                      .arg(QString::fromUtf8(QUrl::toPercentEncoding(user)))
                      .arg(QString::fromUtf8(QUrl::toPercentEncoding(pass)));
    m_socket->open(QUrl(url));
}

void WsClient::disconnect()
{
    m_socket->close();
}

bool WsClient::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

void WsClient::onConnected()
{
    emit connected();
}

void WsClient::onDisconnected()
{
    emit disconnected();
}

void WsClient::onTextMessageReceived(const QString &message)
{
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject()) return;

    QJsonObject root = doc.object();
    QString type = root["type"].toString();
    QJsonObject data = root["data"].toObject();
    emit wsEvent(type, data);
}

void WsClient::onError(QAbstractSocket::SocketError /*error*/)
{
    emit wsError(m_socket->errorString());
}
