#pragma once

#include <QMutex>
#include <QString>

class Logger {
public:
    static Logger& instancia();

    void configurarRuta(const QString& rutaArchivo);
    void registrar(const QString& evento);

private:
    Logger() = default;

    QString m_rutaArchivo;
    QMutex m_mutex;
};
