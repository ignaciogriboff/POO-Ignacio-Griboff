#ifndef CLIMA_SERVICE_H
#define CLIMA_SERVICE_H

#include <QObject>
#include <QNetworkAccessManager>

struct ClimaActual
{
    QString ciudad;
    double temperaturaC = 0.0;
    QString descripcion;
    int timezoneOffsetSec = 0;
    qint64 dtUtc = 0;
};

class ClimaService : public QObject
{
    Q_OBJECT
public:
    explicit ClimaService(QObject *parent = nullptr);

    void fetchClimaActual(const QString &city,
                          const QString &apiKey,
                          const QString &units = "metric");

signals:
    void climaOk(const ClimaActual &clima);
    void climaError(const QString &mensaje);

private:
    QNetworkAccessManager m_nam;
};

#endif // CLIMA_SERVICE_H