#pragma once

#include <QObject>
#include <QUrl>
#include <QNetworkAccessManager>

class DrawingModel;

class SyncService : public QObject {
    Q_OBJECT
public:
    explicit SyncService(const QUrl& baseUrl, QObject* parent = nullptr);

    void loadCanvas(DrawingModel* model);
    void saveCanvas(const DrawingModel* model);

signals:
    void loadFinished(bool ok, QString message);
    void saveFinished(bool ok, QString message);

private:
    QUrl m_baseUrl;
    QNetworkAccessManager m_nam;
};