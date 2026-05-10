#include "login_screen.h"

#include "logger.h"

#include <QCloseEvent>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QResizeEvent>
#include <QVBoxLayout>

LoginScreen::LoginScreen(const AppConfig& config, QWidget* parent)
    : Pantalla(parent)
    , m_config(config)
    , m_intentosFallidos(0)
    , m_titulo(nullptr)
    , m_usuario(nullptr)
    , m_password(nullptr)
    , m_btnIngresar(nullptr)
    , m_btnCerrar(nullptr)
    , m_estado(nullptr) {
    inicializarUI();
    conectarEventos();
    cargarDatos();
    validarEstado();
    registrarEvento("Pantalla Login inicializada");
}

void LoginScreen::inicializarUI() {
    setWindowTitle("Login | Editor Multilenguaje");
    setMinimumSize(480, 380);
    setStyleSheet("background-color: #0a0e1a;");

    auto* outer = new QVBoxLayout(this);
    outer->setAlignment(Qt::AlignCenter);

    auto* card = new QWidget(this);
    card->setObjectName("loginCard");
    card->setFixedWidth(380);
    card->setStyleSheet(
        "QWidget#loginCard {"
        "  background-color: #111827;"
        "  border: 1px solid #1e3a5f;"
        "  border-radius: 8px;"
        "}"
    );

    auto* layout = new QVBoxLayout(card);
    layout->setSpacing(14);
    layout->setContentsMargins(32, 32, 32, 32);

    auto* header = new QHBoxLayout();
    auto* dot1 = new QLabel("●", card);
    dot1->setStyleSheet("color: #00d4ff; font-size: 10px;");
    auto* dot2 = new QLabel("●", card);
    dot2->setStyleSheet("color: #3b82f6; font-size: 10px;");
    auto* dot3 = new QLabel("●", card);
    dot3->setStyleSheet("color: #8b5cf6; font-size: 10px;");
    header->addWidget(dot1);
    header->addWidget(dot2);
    header->addWidget(dot3);
    header->addStretch();
    auto* tag = new QLabel("v1.0", card);
    tag->setStyleSheet("color: #475569; font-size: 10px;");
    header->addWidget(tag);

    m_titulo = new QLabel("EDITOR MULTILENGUAJE", card);
    m_titulo->setStyleSheet(
        "font-size: 20px;"
        "font-weight: 700;"
        "color: #00d4ff;"
        "letter-spacing: 3px;"
        "font-family: 'Consolas', monospace;"
    );
    m_titulo->setAlignment(Qt::AlignHCenter);

    auto* sub = new QLabel("Autenticacion requerida", card);
    sub->setStyleSheet("color: #475569; font-size: 11px; letter-spacing: 1px;");
    sub->setAlignment(Qt::AlignHCenter);

    auto* sep = new QFrame(card);
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("background: #1e3a5f; border: none; max-height: 1px;");

    // Campo usuario con barra lateral de color
    auto* rowUser = new QWidget(card);
    rowUser->setStyleSheet("background: transparent; border-left: 3px solid #00d4ff; padding-left: 8px;");
    auto* rowUserLayout = new QVBoxLayout(rowUser);
    rowUserLayout->setContentsMargins(8, 4, 0, 4);
    rowUserLayout->setSpacing(2);
    auto* labelUser = new QLabel("USUARIO", rowUser);
    labelUser->setStyleSheet("color: #00d4ff; font-size: 9px; letter-spacing: 2px; font-weight: 700; border: none;");
    m_usuario = new QLineEdit(rowUser);
    m_usuario->setPlaceholderText("nombre de usuario");
    m_usuario->setMinimumHeight(34);
    m_usuario->setStyleSheet(
        "QLineEdit { background: #0d1117; border: none; border-bottom: 1px solid #2d3748;"
        "  border-radius: 0; padding: 4px 2px; color: #e2e8f0; font-size: 13px; }"
        "QLineEdit:focus { border-bottom: 1px solid #00d4ff; }"
    );
    rowUserLayout->addWidget(labelUser);
    rowUserLayout->addWidget(m_usuario);

    // Campo password con barra lateral de color
    auto* rowPass = new QWidget(card);
    rowPass->setStyleSheet("background: transparent; border-left: 3px solid #3b82f6; padding-left: 8px;");
    auto* rowPassLayout = new QVBoxLayout(rowPass);
    rowPassLayout->setContentsMargins(8, 4, 0, 4);
    rowPassLayout->setSpacing(2);
    auto* labelPass = new QLabel("CONTRASE\u00d1A", rowPass);
    labelPass->setStyleSheet("color: #3b82f6; font-size: 9px; letter-spacing: 2px; font-weight: 700; border: none;");
    m_password = new QLineEdit(rowPass);
    m_password->setPlaceholderText("contraseña");
    m_password->setEchoMode(QLineEdit::Password);
    m_password->setMinimumHeight(34);
    m_password->setStyleSheet(
        "QLineEdit { background: #0d1117; border: none; border-bottom: 1px solid #2d3748;"
        "  border-radius: 0; padding: 4px 2px; color: #e2e8f0; font-size: 13px; }"
        "QLineEdit:focus { border-bottom: 1px solid #3b82f6; }"
    );
    rowPassLayout->addWidget(labelPass);
    rowPassLayout->addWidget(m_password);

    auto* btnRow = new QHBoxLayout();
    btnRow->setSpacing(10);

    m_btnIngresar = new QPushButton("INGRESAR", card);
    m_btnIngresar->setMinimumHeight(40);
    m_btnIngresar->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #0099bb,stop:1 #1e3a5f);"
        "  border: 1px solid #00d4ff;"
        "  border-radius: 4px;"
        "  color: #fff;"
        "  font-weight: 700;"
        "  letter-spacing: 2px;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #00d4ff,stop:1 #3b82f6);"
        "  color: #0a0e1a;"
        "}"
    );

    m_btnCerrar = new QPushButton("CERRAR", card);
    m_btnCerrar->setMinimumHeight(40);
    m_btnCerrar->setFixedWidth(100);
    m_btnCerrar->setStyleSheet(
        "QPushButton {"
        "  background: transparent;"
        "  border: 1px solid #475569;"
        "  border-radius: 4px;"
        "  color: #475569;"
        "  font-weight: 700;"
        "  letter-spacing: 2px;"
        "}"
        "QPushButton:hover {"
        "  border-color: #ef4444;"
        "  color: #ef4444;"
        "}"
    );

    btnRow->addWidget(m_btnIngresar, 1);
    btnRow->addWidget(m_btnCerrar);

    m_estado = new QLabel("_", card);
    m_estado->setStyleSheet("color: #3b82f6; font-size: 11px; font-family: 'Consolas', monospace;");
    m_estado->setAlignment(Qt::AlignHCenter);

    layout->addLayout(header);
    layout->addWidget(m_titulo);
    layout->addWidget(sub);
    layout->addWidget(sep);
    layout->addSpacing(4);
    layout->addWidget(rowUser);
    layout->addWidget(rowPass);
    layout->addSpacing(8);
    layout->addLayout(btnRow);
    layout->addWidget(m_estado);

    outer->addStretch();
    outer->addWidget(card, 0, Qt::AlignHCenter);
    outer->addStretch();
}

void LoginScreen::conectarEventos() {
    connect(m_btnIngresar, &QPushButton::clicked, this, &LoginScreen::intentarLogin);
    connect(m_password, &QLineEdit::returnPressed, this, &LoginScreen::intentarLogin);
    connect(m_btnCerrar, &QPushButton::clicked, this, &QWidget::close);
}

void LoginScreen::cargarDatos() {
    m_usuario->setText(m_config.initialUser);
}

bool LoginScreen::validarEstado() {
    const bool ok = !m_config.initialUser.isEmpty() && !m_config.initialPassword.isEmpty();
    if (!ok) {
        m_estado->setText("Configuracion invalida: usuario/password vacios");
        m_estado->setStyleSheet("color: #a11d1d;");
    }
    return ok;
}

void LoginScreen::registrarEvento(const QString& descripcion) {
    Logger::instancia().registrar("[Login] " + descripcion);
}

void LoginScreen::intentarLogin() {
    if (!validarEstado()) {
        registrarEvento("Intento de login rechazado por estado invalido");
        return;
    }

    const bool usuarioOk = (m_usuario->text() == m_config.initialUser);
    const bool passOk = (m_password->text() == m_config.initialPassword);

    if (usuarioOk && passOk) {
        m_intentosFallidos = 0;
        m_estado->setText("> Acceso concedido. Iniciando editor...");
        m_estado->setStyleSheet("color: #10b981; font-family: 'Consolas', monospace;");
        registrarEvento("Login exitoso para usuario " + m_usuario->text());
        emit loginCorrecto();
        return;
    }

    ++m_intentosFallidos;
    const int restantes = 3 - m_intentosFallidos;
    registrarEvento("Login fallido. Intento " + QString::number(m_intentosFallidos));

    if (m_intentosFallidos >= 3) {
        m_estado->setText("> ERROR: demasiados intentos. Sistema bloqueando...");
        m_estado->setStyleSheet("color: #ef4444; font-family: 'Consolas', monospace;");
        m_intentosFallidos = 0;
        emit bloqueoSolicitado();
    } else {
        m_estado->setText("> ACCESO DENEGADO  intentos restantes: " + QString::number(restantes));
        m_estado->setStyleSheet("color: #ef4444; font-family: 'Consolas', monospace;");
    }
}

void LoginScreen::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        registrarEvento("Atajo Enter capturado en Login");
        intentarLogin();
        return;
    }
    Pantalla::keyPressEvent(event);
}

void LoginScreen::mousePressEvent(QMouseEvent* event) {
    registrarEvento("Click en Login para foco de usuario");
    if (m_usuario) {
        m_usuario->setFocus();
    }
    Pantalla::mousePressEvent(event);
}

void LoginScreen::resizeEvent(QResizeEvent* event) {
    registrarEvento("Resize Login a " + QString::number(event->size().width()) + "x" + QString::number(event->size().height()));
    Pantalla::resizeEvent(event);
}

void LoginScreen::closeEvent(QCloseEvent* event) {
    const auto respuesta = QMessageBox::question(this, "Salir", "Desea cerrar la aplicacion?");
    if (respuesta == QMessageBox::Yes) {
        registrarEvento("Cierre aceptado desde Login");
        event->accept();
    } else {
        registrarEvento("Cierre cancelado desde Login");
        event->ignore();
    }
}

void LoginScreen::focusInEvent(QFocusEvent* event) {
    registrarEvento("Login obtuvo foco");
    Pantalla::focusInEvent(event);
}

void LoginScreen::focusOutEvent(QFocusEvent* event) {
    registrarEvento("Login perdio foco");
    Pantalla::focusOutEvent(event);
}
