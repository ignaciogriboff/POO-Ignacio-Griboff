#include "app_config.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QSettings>

QString AppConfig::s_configPath;

static QString tryPath(const QString &path)
{
    return QFileInfo::exists(path) ? QFileInfo(path).absoluteFilePath() : QString();
}

static QString findConfigNearWorkingDir()
{
    // Working dir (Qt Creator lo controla en Projects > Run)
    const QString wd = QDir::currentPath();

    // Buscamos en ./, ../, ../../, ../../../
    QString p;

    p = tryPath(wd + "/config.ini");
    if (!p.isEmpty()) return p;

    p = tryPath(wd + "/../config.ini");
    if (!p.isEmpty()) return p;

    p = tryPath(wd + "/../../config.ini");
    if (!p.isEmpty()) return p;

    p = tryPath(wd + "/../../../config.ini");
    if (!p.isEmpty()) return p;

    return QString();
}

static QString findConfigFallbackAppDir()
{
    const QString appDir = QCoreApplication::applicationDirPath();
    return tryPath(appDir + "/config.ini");
}

void AppConfig::init()
{
    // 1) Preferimos proyecto (working dir / parents)
    s_configPath = findConfigNearWorkingDir();

    // 2) Fallback: al lado del exe
    if (s_configPath.isEmpty())
        s_configPath = findConfigFallbackAppDir();

    // 3) Si no existe, igual dejamos un path “esperado” en el proyecto
    if (s_configPath.isEmpty())
        s_configPath = QDir::currentPath() + "/config.ini";
}

QString AppConfig::configPath()
{
    if (s_configPath.isEmpty())
        init();
    return s_configPath;
}

static QSettings settings()
{
    return QSettings(AppConfig::configPath(), QSettings::IniFormat);
}

// Weather
QString AppConfig::weatherCity()
{
    QSettings s = settings();
    return s.value("weather/city", "Cordoba,AR").toString();
}

QString AppConfig::weatherUnits()
{
    QSettings s = settings();
    return s.value("weather/units", "metric").toString();
}

QString AppConfig::weatherApiKey()
{
    QSettings s = settings();
    return s.value("weather/apikey", "").toString();
}

// Images
QUrl AppConfig::loginBgUrl()
{
    QSettings s = settings();
    return QUrl(s.value("images/login_bg_url", "").toString());
}

QUrl AppConfig::mainBgUrl()
{
    QSettings s = settings();
    return QUrl(s.value("images/main_bg_url", "").toString());
}

// Proxy
bool AppConfig::proxyUseSystem()
{
    QSettings s = settings();
    return s.value("proxy/use_system", true).toBool();
}

QString AppConfig::proxyType()
{
    QSettings s = settings();
    return s.value("proxy/type", "http").toString();
}

QString AppConfig::proxyHost()
{
    QSettings s = settings();
    return s.value("proxy/host", "").toString();
}

int AppConfig::proxyPort()
{
    QSettings s = settings();
    return s.value("proxy/port", 8080).toInt();
}

QString AppConfig::proxyUser()
{
    QSettings s = settings();
    return s.value("proxy/user", "").toString();
}

QString AppConfig::proxyPass()
{
    QSettings s = settings();
    return s.value("proxy/pass", "").toString();
}