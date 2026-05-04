#include "api_client.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QUrl>
#include <QByteArray>

ApiClient::ApiClient(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{}

void ApiClient::setConfig(const QString &baseUrl, const QString &user, const QString &pass)
{
    m_baseUrl = baseUrl;
    m_user = user;
    m_pass = pass;
}

// Build a QNetworkRequest with Basic Auth and JSON content type
QNetworkRequest ApiClient::makeRequest(const QString &path) const
{
    QNetworkRequest req(QUrl(m_baseUrl + path));
    QString credentials = m_user + ":" + m_pass;
    QByteArray encoded = credentials.toUtf8().toBase64();
    req.setRawHeader("Authorization", "Basic " + encoded);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    return req;
}

// Generic reply handler: checks HTTP errors, calls handler on success
void ApiClient::handleReply(QNetworkReply *reply, std::function<void(const QByteArray &)> handler)
{
    connect(reply, &QNetworkReply::finished, this, [this, reply, handler]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            QString msg = QString("HTTP %1: %2").arg(statusCode).arg(reply->errorString());
            emit errorOccurred(msg);
            return;
        }
        handler(reply->readAll());
    });
}

// ── Columns ───────────────────────────────────────────────────────────────────

void ApiClient::fetchColumns()
{
    auto *reply = m_nam->get(makeRequest("/columns"));
    handleReply(reply, [this](const QByteArray &data) {
        QJsonArray arr = QJsonDocument::fromJson(data).array();
        emit columnsFetched(parseColumns(arr));
    });
}

void ApiClient::createColumn(const QString &title)
{
    QJsonObject body;
    body["title"] = title;
    auto *reply = m_nam->post(makeRequest("/columns"), QJsonDocument(body).toJson());
    handleReply(reply, [this](const QByteArray &data) {
        emit columnCreated(parseColumn(QJsonDocument::fromJson(data).object()));
    });
}

void ApiClient::updateColumn(int id, const QString &title, int position)
{
    QJsonObject body;
    body["title"] = title;
    body["position"] = position;
    auto *reply = m_nam->put(makeRequest(QString("/columns/%1").arg(id)),
                              QJsonDocument(body).toJson());
    handleReply(reply, [this](const QByteArray &data) {
        emit columnUpdated(parseColumn(QJsonDocument::fromJson(data).object()));
    });
}

void ApiClient::deleteColumn(int id)
{
    auto *reply = m_nam->deleteResource(makeRequest(QString("/columns/%1").arg(id)));
    handleReply(reply, [this, id](const QByteArray &) {
        emit columnDeleted(id);
    });
}

// ── Cards ─────────────────────────────────────────────────────────────────────

void ApiClient::createCard(int columnId, const QString &title, const QString &description)
{
    QJsonObject body;
    body["column_id"] = columnId;
    body["title"] = title;
    body["description"] = description;
    auto *reply = m_nam->post(makeRequest("/cards"), QJsonDocument(body).toJson());
    handleReply(reply, [this](const QByteArray &data) {
        emit cardCreated(parseCard(QJsonDocument::fromJson(data).object()));
    });
}

void ApiClient::updateCard(int id, const QString &title, const QString &description)
{
    QJsonObject body;
    body["title"] = title;
    body["description"] = description;
    auto *reply = m_nam->put(makeRequest(QString("/cards/%1").arg(id)),
                              QJsonDocument(body).toJson());
    handleReply(reply, [this](const QByteArray &data) {
        emit cardUpdated(parseCard(QJsonDocument::fromJson(data).object()));
    });
}

void ApiClient::deleteCard(int id)
{
    auto *reply = m_nam->deleteResource(makeRequest(QString("/cards/%1").arg(id)));
    handleReply(reply, [this, id](const QByteArray &) {
        emit cardDeleted(id);
    });
}

void ApiClient::moveCard(int cardId, int toColumnId, int toPosition)
{
    QJsonObject body;
    body["to_column_id"] = toColumnId;
    body["to_position"] = toPosition;
    auto *reply = m_nam->post(makeRequest(QString("/cards/%1/move").arg(cardId)),
                               QJsonDocument(body).toJson());
    handleReply(reply, [this](const QByteArray &data) {
        emit cardMoved(parseCard(QJsonDocument::fromJson(data).object()));
    });
}

void ApiClient::reorderCard(int columnId, int cardId, int toPosition)
{
    QJsonObject body;
    body["card_id"] = cardId;
    body["to_position"] = toPosition;
    auto *reply = m_nam->post(makeRequest(QString("/columns/%1/reorder").arg(columnId)),
                               QJsonDocument(body).toJson());
    handleReply(reply, [this, columnId, cardId, toPosition](const QByteArray &) {
        emit cardReordered(columnId, cardId, toPosition);
    });
}

// ── Parsers ───────────────────────────────────────────────────────────────────

QList<Column> ApiClient::parseColumns(const QJsonArray &arr)
{
    QList<Column> result;
    for (const auto &val : arr) {
        result.append(parseColumn(val.toObject()));
    }
    return result;
}

Column ApiClient::parseColumn(const QJsonObject &obj)
{
    Column col;
    col.id = obj["id"].toInt();
    col.title = obj["title"].toString();
    col.position = obj["position"].toInt();
    for (const auto &cardVal : obj["cards"].toArray()) {
        col.cards.append(parseCard(cardVal.toObject()));
    }
    // Ensure cards are sorted by position
    std::sort(col.cards.begin(), col.cards.end(),
              [](const Card &a, const Card &b) { return a.position < b.position; });
    return col;
}

Card ApiClient::parseCard(const QJsonObject &obj)
{
    Card card;
    card.id = obj["id"].toInt();
    card.columnId = obj["column_id"].toInt();
    card.title = obj["title"].toString();
    card.description = obj["description"].toString();
    card.position = obj["position"].toInt();
    return card;
}
