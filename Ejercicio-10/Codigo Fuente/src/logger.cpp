#include "logger.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

Logger& Logger::instancia() {
    static Logger instance;
    return instance;
}

void Logger::configurarRuta(const QString& rutaArchivo) {
    QMutexLocker lock(&m_mutex);
    m_rutaArchivo = rutaArchivo;

    QFileInfo info(m_rutaArchivo);
    QDir().mkpath(info.absolutePath());
}

void Logger::registrar(const QString& evento) {
    QMutexLocker lock(&m_mutex);

    if (m_rutaArchivo.isEmpty()) {
        return;
    }

    QFile archivo(m_rutaArchivo);
    if (!archivo.open(QIODevice::Append | QIODevice::Text)) {
        return;
    }

    QTextStream out(&archivo);
    out << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")
        << " | " << evento << "\n";
}
