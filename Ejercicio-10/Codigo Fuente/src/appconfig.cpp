#include "appconfig.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSettings>

AppConfig AppConfig::cargar(const QString& preferredPath) {
    QString configPath = preferredPath;
    if (!QFile::exists(configPath)) {
        const QString candidate = QDir(QCoreApplication::applicationDirPath()).filePath("../config.ini");
        if (QFile::exists(candidate)) {
            configPath = candidate;
        }
    }

    QSettings settings(configPath, QSettings::IniFormat);

    AppConfig config;
    config.initialUser = settings.value("auth/initial_user", "admin").toString();
    config.initialPassword = settings.value("auth/initial_password", "1234").toString();
    config.lockSeconds = settings.value("auth/lock_seconds", 20).toInt();
    config.defaultLanguage = settings.value("editor/default_language", "Python").toString();
    config.exportPath = settings.value("editor/export_path", "exports/codigo_exportado.jpg").toString();

    return config;
}
