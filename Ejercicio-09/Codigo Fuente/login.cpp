#include "login.h"
#include "ui_login.h"

#include "logger.h"
#include "app_config.h"

#include <QDateTime>
#include <QPalette>
#include <QResizeEvent>
#include <QUrl>

Login::Login(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Login)
{
    ui->setupUi(this);

    ui->lblStatus->clear();
    ui->lblClima->setText("Cargando clima...");
    ui->lblHora->setText("Hora local: --:--");

    Logger::log("LOGIN", "Pantalla Login creada.");

    // --- Login ---
    connect(ui->lePass, &QLineEdit::returnPressed, this, &Login::onLoginClicked);
    connect(ui->btnLogin, &QPushButton::clicked, this, &Login::onLoginClicked);

    m_lockTimer.setInterval(1000);
    connect(&m_lockTimer, &QTimer::timeout, this, &Login::onLockTick);

    setLocked(false);

    // --- Reloj local (sistema) ---
    m_clockTimer.setInterval(1000);
    connect(&m_clockTimer, &QTimer::timeout, this, [this]() {
        const QString hhmmss = QDateTime::currentDateTime().toString("HH:mm:ss");
        ui->lblHora->setText(QString("Hora local: %1").arg(hhmmss));
    });
    m_clockTimer.start();

    // --- Clima ---
    connect(&m_clima, &ClimaService::climaOk, this, [this](const ClimaActual &c) {
        ui->lblClima->setText(QString("%1: %2 °C (%3)")
                                  .arg(c.ciudad)
                                  .arg(c.temperaturaC, 0, 'f', 1)
                                  .arg(c.descripcion));

        // Hora local usando dt (UTC) + timezone offset (segundos)
        const qint64 localEpoch = c.dtUtc + c.timezoneOffsetSec;
        const QDateTime localTime = QDateTime::fromSecsSinceEpoch(localEpoch, Qt::UTC);
        ui->lblHora->setText(QString("Hora local (API): %1").arg(localTime.toString("HH:mm:ss")));

        Logger::log("CLIMA", QString("OK %1C %2").arg(c.temperaturaC, 0, 'f', 1).arg(c.descripcion));
    });

    connect(&m_clima, &ClimaService::climaError, this, [this](const QString &msg) {
        ui->lblClima->setText(QString("Clima (offline): 21.0 °C (simulado) — %1").arg(msg));

        const QString hhmmss = QDateTime::currentDateTime().toString("HH:mm:ss");
        ui->lblHora->setText(QString("Hora local (offline): %1").arg(hhmmss));

        Logger::log("CLIMA", "ERROR " + msg);
    });

    // --- Clima desde config.ini ---
    const QString city = AppConfig::weatherCity();
    const QString units = AppConfig::weatherUnits();
    const QString apiKey = AppConfig::weatherApiKey();

    Logger::log("CLIMA", "Fetch clima actual: " + city + " units=" + units);
    m_clima.fetchClimaActual(city, apiKey, units);

    // --- Fondo: descarga + cache (desde config.ini) ---
    connect(&m_img, &ImageCacheService::imageReady, this,
            [this](const QPixmap &pm, const QString &cachePath) {
                m_bgOriginal = pm;
                applyBackground(pm);
                Logger::log("IMG", "Fondo login aplicado desde: " + cachePath);
            });

    connect(&m_img, &ImageCacheService::imageError, this, [this](const QString &msg) {
        ui->lblStatus->setText(QString("Fondo: %1").arg(msg));
        Logger::log("IMG", "ERROR fondo login: " + msg);
    });

    const QUrl bgUrl = AppConfig::loginBgUrl();
    Logger::log("IMG", "Fetch fondo login: " + bgUrl.toString());
    m_img.fetchImage(bgUrl, "login_bg");
}

Login::~Login()
{
    Logger::log("LOGIN", "Pantalla Login destruida.");
    delete ui;
}

void Login::setStatusText(const QString &text)
{
    ui->lblStatus->setText(text);
}

void Login::applyBackground(const QPixmap &pm)
{
    if (pm.isNull()) return;

    // Escalado sin deformar (cubre toda la ventana, puede recortar)
    QPixmap scaled = pm.scaled(size(),
                               Qt::KeepAspectRatioByExpanding,
                               Qt::SmoothTransformation);

    QPalette pal = palette();
    pal.setBrush(QPalette::Window, scaled);
    setAutoFillBackground(true);
    setPalette(pal);
}

void Login::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (!m_bgOriginal.isNull())
        applyBackground(m_bgOriginal);
}

void Login::showStatus(const QString &text, const QString &color)
{
    ui->lblStatus->setText(text);
    ui->lblStatus->setStyleSheet(QString("color: %1;").arg(color));
}

void Login::setLocked(bool locked)
{
    m_locked = locked;
    ui->leUser->setEnabled(!locked);
    ui->lePass->setEnabled(!locked);
    ui->btnLogin->setEnabled(!locked);

    if (!locked) ui->leUser->setFocus();
}

void Login::onLoginClicked()
{
    if (m_locked) return;

    const QString user = ui->leUser->text().trimmed();
    const QString pass = ui->lePass->text();

    const bool ok = (user == "admin" && pass == "1234");

    if (ok) {
        m_failedAttempts = 0;
        Logger::log("LOGIN", QString("Login OK. user='%1'").arg(user));
        showStatus("Login correcto.", "lightgreen");
        emit loginOk();
        return;
    }

    m_failedAttempts++;
    Logger::log("LOGIN", QString("Falló login. user='%1' intento=%2")
                             .arg(user).arg(m_failedAttempts));

    const int remaining = kMaxAttempts - m_failedAttempts;
    if (remaining > 0) {
        showStatus(QString("Usuario o contraseña incorrectos. Intentos restantes: %1").arg(remaining),
                   "salmon");
        return;
    }

    Logger::log("LOGIN", "Bloqueo activado por 3 intentos fallidos (10s).");
    m_lockRemainingSec = kLockSeconds;
    setLocked(true);
    showStatus(QString("Demasiados intentos. Bloqueado por %1s...").arg(m_lockRemainingSec),
               "orange");
    m_lockTimer.start();
}

void Login::onLockTick()
{
    m_lockRemainingSec--;

    if (m_lockRemainingSec <= 0) {
        m_lockTimer.stop();
        m_failedAttempts = 0;
        setLocked(false);
        Logger::log("LOGIN", "Bloqueo finalizado. Login habilitado nuevamente.");
        showStatus("Podés volver a intentar.", "lightgreen");
        ui->lePass->clear();
        return;
    }

    showStatus(QString("Demasiados intentos. Bloqueado por %1s...").arg(m_lockRemainingSec),
               "orange");
}