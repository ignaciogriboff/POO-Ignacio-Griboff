#include "application_controller.h"

#include "blocked_screen.h"
#include "editor_screen.h"
#include "logger.h"
#include "login_screen.h"
#include "pantalla.h"

ApplicationController::ApplicationController(const AppConfig& config, QObject* parent)
    : QObject(parent)
    , m_config(config)
    , m_pantallaActual(nullptr) {
}

ApplicationController::~ApplicationController() {
    if (m_pantallaActual) {
        m_pantallaActual->close();
        delete m_pantallaActual;
        m_pantallaActual = nullptr;
    }
}

void ApplicationController::iniciar() {
    mostrarLogin();
}

void ApplicationController::mostrarLogin() {
    auto* login = new LoginScreen(m_config);
    connect(login, &LoginScreen::loginCorrecto, this, &ApplicationController::mostrarEditor);
    connect(login, &LoginScreen::bloqueoSolicitado, this, &ApplicationController::mostrarBloqueado);
    cambiarPantalla(login);
}

void ApplicationController::mostrarBloqueado() {
    auto* bloqueo = new BlockedScreen(m_config.lockSeconds);
    connect(bloqueo, &BlockedScreen::bloqueoFinalizado, this, &ApplicationController::mostrarLogin);
    cambiarPantalla(bloqueo);
}

void ApplicationController::mostrarEditor() {
    auto* editor = new EditorScreen(m_config.defaultLanguage, m_config.exportPath);
    cambiarPantalla(editor);
    editor->showFullScreen();
    Logger::instancia().registrar("[Flow] Login valido: editor abierto en pantalla completa");
}

void ApplicationController::cambiarPantalla(Pantalla* nuevaPantalla) {
    if (m_pantallaActual) {
        m_pantallaActual->hide();
        m_pantallaActual->deleteLater();
    }

    m_pantallaActual = nuevaPantalla;
    if (m_pantallaActual) {
        m_pantallaActual->show();
    }
}
