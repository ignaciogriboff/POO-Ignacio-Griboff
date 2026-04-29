#include "logger.h"

#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QTextStream>

QString Logger::s_logFilePath;

void Logger::init(const QString &appName)
{
    // Guardar en AppDataLocation/logs/app.log
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QString dir = base + "/logs";
    QDir().mkpath(dir);

    s_logFilePath = dir + "/" + appName + ".log";

    // Crear archivo si no existe (opcional)
    QFile f(s_logFilePath);
    if (f.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&f);
        out << "----- Logger started: "
            << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")
            << " -----\n";
    }
}

QString Logger::logFilePath()
{
    return s_logFilePath;
}

void Logger::log(const QString &message)
{
    if (s_logFilePath.isEmpty()) {
        // fallback: inicializa con default si se olvidaron de init
        init("Ej07");
    }

    QFile f(s_logFilePath);
    if (!f.open(QIODevice::Append | QIODevice::Text))
        return;

    QTextStream out(&f);
    out << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")
        << " | " << message << "\n";
}

void Logger::log(const QString &tag, const QString &message)
{
    log(QString("[%1] %2").arg(tag, message));
}