#include <QApplication>
#include <QNetworkProxyFactory>
#include <QNetworkProxy>
#include <QUrl>

#include "logger.h"
#include "app_config.h"
#include "login.h"
#include "ventana.h"
#include "image_cache_service.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Logger::init("Ej07");
    Logger::log("APP", "Aplicación iniciada. Log file: " + Logger::logFilePath());

    AppConfig::init();
    Logger::log("CFG", "Config path: " + AppConfig::configPath());

    // Proxy desde config
    if (AppConfig::proxyUseSystem()) {
        QNetworkProxyFactory::setUseSystemConfiguration(true);
        Logger::log("NET", "Proxy: system");
    } else {
        QNetworkProxy proxy;
        proxy.setType(QNetworkProxy::HttpProxy);
        proxy.setHostName(AppConfig::proxyHost());
        proxy.setPort(static_cast<quint16>(AppConfig::proxyPort()));
        proxy.setUser(AppConfig::proxyUser());
        proxy.setPassword(AppConfig::proxyPass());
        QNetworkProxy::setApplicationProxy(proxy);
        Logger::log("NET", "Proxy: manual " + AppConfig::proxyHost() + ":" + QString::number(AppConfig::proxyPort()));
    }

    Login login;
    Ventana ventana;
    ImageCacheService imgMain;

    QObject::connect(&login, &Login::loginOk, [&]() {
        Logger::log("APP", "Login OK. Descargando imagen principal...");
        login.setStatusText("Login OK. Descargando imagen principal...");

        const QUrl mainBgUrl = AppConfig::mainBgUrl();
        Logger::log("IMG", "Fetch fondo principal: " + mainBgUrl.toString());
        imgMain.fetchImage(mainBgUrl, "main_bg");
    });

    QObject::connect(&imgMain, &ImageCacheService::imageReady,
                     [&](const QPixmap &pm, const QString &cachePath) {
                         Logger::log("APP", "Imagen principal lista: " + cachePath);

                         ventana.setBackground(pm);
                         ventana.showFullScreen();
                         Logger::log("APP", "Ventana principal mostrada en fullscreen.");

                         login.hide();
                         Logger::log("APP", "Login ocultado.");
                     });

    QObject::connect(&imgMain, &ImageCacheService::imageError, [&](const QString &msg) {
        Logger::log("APP", "ERROR descargando imagen principal: " + msg);
        login.setStatusText("Error descargando imagen principal: " + msg);
    });

    login.show();
    Logger::log("APP", "Login mostrado.");

    return a.exec();
}