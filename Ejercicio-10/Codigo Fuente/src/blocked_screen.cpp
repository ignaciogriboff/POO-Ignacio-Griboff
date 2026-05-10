#include "blocked_screen.h"

#include "logger.h"

#include <QCloseEvent>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QProgressBar>
#include <QResizeEvent>
#include <QTimer>
#include <QVBoxLayout>

BlockedScreen::BlockedScreen(int lockSeconds, QWidget* parent)
    : Pantalla(parent)
    , m_lockSeconds(lockSeconds)
    , m_remaining(lockSeconds)
    , m_titulo(nullptr)
    , m_detalle(nullptr)
    , m_progress(nullptr)
    , m_timer(nullptr) {
    inicializarUI();
    conectarEventos();
    cargarDatos();
    validarEstado();
    registrarEvento("Pantalla Bloqueado inicializada");
}

void BlockedScreen::inicializarUI() {
    setWindowTitle("SISTEMA BLOQUEADO");
    setMinimumSize(480, 320);
    setStyleSheet("background-color: #0a0e1a;");

    auto* outer = new QVBoxLayout(this);
    outer->setAlignment(Qt::AlignCenter);

    auto* card = new QWidget(this);
    card->setFixedWidth(400);
    card->setStyleSheet(
        "background-color: #111827;"
        "border: 1px solid #7f1d1d;"
        "border-radius: 8px;"
    );

    auto* layout = new QVBoxLayout(card);
    layout->setSpacing(16);
    layout->setContentsMargins(36, 36, 36, 36);

    auto* icon = new QLabel("⚠", card);
    icon->setStyleSheet("color: #ef4444; font-size: 36px;");
    icon->setAlignment(Qt::AlignHCenter);

    m_titulo = new QLabel("ACCESO BLOQUEADO", card);
    m_titulo->setStyleSheet(
        "font-size: 18px;"
        "font-weight: 700;"
        "color: #ef4444;"
        "letter-spacing: 3px;"
        "font-family: 'Consolas', monospace;"
    );
    m_titulo->setAlignment(Qt::AlignHCenter);

    auto* sep = new QFrame(card);
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("background: #7f1d1d; border: none; max-height: 1px;");

    m_detalle = new QLabel(card);
    m_detalle->setStyleSheet(
        "color: #94a3b8;"
        "font-family: 'Consolas', monospace;"
        "font-size: 13px;"
    );
    m_detalle->setAlignment(Qt::AlignHCenter);

    m_progress = new QProgressBar(card);
    m_progress->setRange(0, m_lockSeconds);
    m_progress->setTextVisible(false);
    m_progress->setFixedHeight(6);
    m_progress->setStyleSheet(
        "QProgressBar { background: #1f2937; border: none; border-radius: 3px; }"
        "QProgressBar::chunk { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #ef4444, stop:1 #f97316); border-radius: 3px; }"
    );

    layout->addWidget(icon);
    layout->addWidget(m_titulo);
    layout->addWidget(sep);
    layout->addWidget(m_detalle);
    layout->addWidget(m_progress);

    outer->addStretch();
    outer->addWidget(card, 0, Qt::AlignHCenter);
    outer->addStretch();

    m_timer = new QTimer(this);
}

void BlockedScreen::conectarEventos() {
    connect(m_timer, &QTimer::timeout, this, &BlockedScreen::actualizarCuentaRegresiva);
}

void BlockedScreen::cargarDatos() {
    m_remaining = m_lockSeconds;
    m_progress->setValue(0);
    m_detalle->setText("Reintente en " + QString::number(m_remaining) + " segundos");
    m_timer->start(1000);
}

bool BlockedScreen::validarEstado() {
    if (m_lockSeconds <= 0) {
        m_lockSeconds = 10;
    }
    return true;
}

void BlockedScreen::registrarEvento(const QString& descripcion) {
    Logger::instancia().registrar("[Bloqueado] " + descripcion);
}

void BlockedScreen::actualizarCuentaRegresiva() {
    --m_remaining;
    if (m_remaining < 0) {
        m_remaining = 0;
    }

    m_detalle->setText("Reintente en " + QString::number(m_remaining) + " segundos");
    m_progress->setValue(m_lockSeconds - m_remaining);

    if (m_remaining == 0) {
        m_timer->stop();
        registrarEvento("Bloqueo finalizado");
        emit bloqueoFinalizado();
    }
}

void BlockedScreen::keyPressEvent(QKeyEvent* event) {
    registrarEvento("Tecla ignorada en modo bloqueado: " + QString::number(event->key()));
    event->accept();
}

void BlockedScreen::mousePressEvent(QMouseEvent* event) {
    registrarEvento("Click ignorado en modo bloqueado");
    event->accept();
}

void BlockedScreen::resizeEvent(QResizeEvent* event) {
    registrarEvento("Resize Bloqueado a " + QString::number(event->size().width()) + "x" + QString::number(event->size().height()));
    Pantalla::resizeEvent(event);
}

void BlockedScreen::closeEvent(QCloseEvent* event) {
    const auto respuesta = QMessageBox::question(this, "Cerrar", "El sistema esta bloqueado. Desea cerrar igual?");
    if (respuesta == QMessageBox::Yes) {
        registrarEvento("Cierre aceptado en modo bloqueado");
        event->accept();
    } else {
        registrarEvento("Cierre cancelado en modo bloqueado");
        event->ignore();
    }
}

void BlockedScreen::focusInEvent(QFocusEvent* event) {
    registrarEvento("Pantalla bloqueada obtuvo foco");
    Pantalla::focusInEvent(event);
}

void BlockedScreen::focusOutEvent(QFocusEvent* event) {
    registrarEvento("Pantalla bloqueada perdio foco");
    Pantalla::focusOutEvent(event);
}
