#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include "models.h"

/**
 * ApiClient wraps all REST calls to the Kanban backend.
 * Authentication: HTTP Basic Auth via Authorization header.
 * All async methods emit corresponding signals on completion or error.
 */
class ApiClient : public QObject
{
    Q_OBJECT
public:
    explicit ApiClient(QObject *parent = nullptr);

    void setConfig(const QString &baseUrl, const QString &user, const QString &pass);

    // Columns
    void fetchColumns();
    void createColumn(const QString &title);
    void updateColumn(int id, const QString &title, int position);
    void deleteColumn(int id);

    // Cards
    void createCard(int columnId, const QString &title, const QString &description);
    void updateCard(int id, const QString &title, const QString &description);
    void deleteCard(int id);
    void moveCard(int cardId, int toColumnId, int toPosition);
    void reorderCard(int columnId, int cardId, int toPosition);

signals:
    void columnsFetched(const QList<Column> &columns);
    void columnCreated(const Column &col);
    void columnUpdated(const Column &col);
    void columnDeleted(int id);

    void cardCreated(const Card &card);
    void cardUpdated(const Card &card);
    void cardDeleted(int id);
    void cardMoved(const Card &card);
    void cardReordered(int columnId, int cardId, int toPosition);

    void errorOccurred(const QString &message);

private:
    QNetworkAccessManager *m_nam;
    QString m_baseUrl;
    QString m_user;
    QString m_pass;

    QNetworkRequest makeRequest(const QString &path) const;
    void handleReply(QNetworkReply *reply, std::function<void(const QByteArray &)> handler);

    static QList<Column> parseColumns(const QJsonArray &arr);
    static Column parseColumn(const QJsonObject &obj);
    static Card parseCard(const QJsonObject &obj);
};
