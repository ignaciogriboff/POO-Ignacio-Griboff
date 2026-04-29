#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <QString>
#include <QUrl>

class AppConfig
{
public:
    static void init();
    static QString configPath();

    // Weather
    static QString weatherCity();
    static QString weatherUnits();
    static QString weatherApiKey();

    // Images
    static QUrl loginBgUrl();
    static QUrl mainBgUrl();

    // Proxy
    static bool proxyUseSystem();
    static QString proxyType();   // "http"
    static QString proxyHost();
    static int proxyPort();
    static QString proxyUser();
    static QString proxyPass();

private:
    static QString s_configPath;
};

#endif // APP_CONFIG_H