#pragma once

#include <QString>

struct AppConfig {
    QString initialUser;
    QString initialPassword;
    int lockSeconds;
    QString defaultLanguage;
    QString exportPath;

    static AppConfig cargar(const QString& preferredPath);
};
