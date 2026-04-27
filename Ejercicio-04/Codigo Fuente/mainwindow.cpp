#include "mainwindow.h"

#include <QCloseEvent>
#include <QDateTime>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollBar>
#include <QSettings>
#include <QSpinBox>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>
#include <QApplication>
#include <QPalette>

static const char* COLOR_BG      = "#0b1220";   // fondo
static const char* COLOR_PANEL   = "#111a2e";   // panel/card
static const char* COLOR_BORDER  = "#22304f";   // borde
static const char* COLOR_TEXT    = "#d6deeb";   // texto
static const char* COLOR_MUTED   = "#93a4c7";   // texto secundario

static const char* COLOR_OK      = "#2ecc71";
static const char* COLOR_WARN    = "#f1c40f";
static const char* COLOR_DOWN    = "#e74c3c";

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    applyGrafanaDarkTheme();

    setWindowTitle("VPS Monitor Panel");
    resize(1100, 620);

    auto *central = new QWidget(this);
    setCentralWidget(central);

    auto *root = new QVBoxLayout(central);
    root->setContentsMargins(14, 14, 14, 14);
    root->setSpacing(12);

    // ===== Header: título + estado badge =====
    auto *header = new QHBoxLayout();
    header->setSpacing(12);

    auto *lblTitle = new QLabel("VPS Monitoring Dashboard", central);
    QFont titleFont = lblTitle->font();
    titleFont.setPointSize(titleFont.pointSize() + 6);
    titleFont.setBold(true);
    lblTitle->setFont(titleFont);
    lblTitle->setStyleSheet(QString("color:%1;").arg(COLOR_TEXT));

    m_lblStatusBadge = new QLabel("—", central);
    m_lblStatusBadge->setAlignment(Qt::AlignCenter);
    m_lblStatusBadge->setFixedHeight(30);
    m_lblStatusBadge->setMinimumWidth(120);
    QFont badgeFont = m_lblStatusBadge->font();
    badgeFont.setBold(true);
    m_lblStatusBadge->setFont(badgeFont);

    m_lblStatusDetail = new QLabel("", central);
    m_lblStatusDetail->setStyleSheet(QString("color:%1;").arg(COLOR_MUTED));
    m_lblStatusDetail->setTextInteractionFlags(Qt::TextSelectableByMouse);

    header->addWidget(lblTitle, 1);
    header->addWidget(m_lblStatusDetail, 2);
    header->addWidget(m_lblStatusBadge, 0);

    root->addLayout(header);

    // ===== Config (a la izquierda) + grid de métricas (derecha) =====
    auto *topRow = new QHBoxLayout();
    topRow->setSpacing(12);

    // ----- Config content -----
    auto *configWidget = new QWidget(central);
    auto *configLayout = new QVBoxLayout(configWidget);
    configLayout->setContentsMargins(0,0,0,0);
    configLayout->setSpacing(10);

    // Conexión
    auto *connBox = new QGroupBox("Conexión", configWidget);
    auto *connForm = new QFormLayout(connBox);
    connForm->setLabelAlignment(Qt::AlignLeft);
    connForm->setFormAlignment(Qt::AlignTop);
    connForm->setHorizontalSpacing(10);
    connForm->setVerticalSpacing(8);

    m_leUrl = new QLineEdit(connBox);
    m_leUrl->setText("http://173.212.234.190:8080/health");

    m_sbInterval = new QSpinBox(connBox);
    m_sbInterval->setRange(1, 3600);
    m_sbInterval->setValue(10);
    m_sbInterval->setSuffix(" s");

    m_btnRefresh = new QPushButton("Refrescar ahora", connBox);

    connForm->addRow("Endpoint URL", m_leUrl);
    connForm->addRow("Intervalo", m_sbInterval);
    connForm->addRow("", m_btnRefresh);

    // Umbrales
    auto *thrBox = new QGroupBox("Umbrales (ALERTA)", configWidget);
    auto *thrForm = new QFormLayout(thrBox);
    thrForm->setHorizontalSpacing(10);
    thrForm->setVerticalSpacing(8);

    m_sbLoadWarn = new QDoubleSpinBox(thrBox);
    m_sbLoadWarn->setRange(0.1, 100.0);
    m_sbLoadWarn->setDecimals(2);
    m_sbLoadWarn->setValue(2.0);

    m_sbMemWarn = new QSpinBox(thrBox);
    m_sbMemWarn->setRange(0, 100);
    m_sbMemWarn->setValue(80);
    m_sbMemWarn->setSuffix(" %");

    m_sbDiskWarn = new QSpinBox(thrBox);
    m_sbDiskWarn->setRange(0, 100);
    m_sbDiskWarn->setValue(85);
    m_sbDiskWarn->setSuffix(" %");

    thrForm->addRow("Load 1m >=", m_sbLoadWarn);
    thrForm->addRow("RAM >=", m_sbMemWarn);
    thrForm->addRow("Disco >=", m_sbDiskWarn);

    // Info de último check
    auto *infoBox = new QGroupBox("Chequeo", configWidget);
    auto *infoForm = new QFormLayout(infoBox);
    m_lblLastCheck = new QLabel("—", infoBox);
    m_lblLastCheck->setStyleSheet(QString("color:%1;").arg(COLOR_TEXT));
    infoForm->addRow("Último chequeo", m_lblLastCheck);

    configLayout->addWidget(connBox);
    configLayout->addWidget(thrBox);
    configLayout->addWidget(infoBox);
    configLayout->addStretch(1);

    // Wrap config in card
    auto *configCard = createCard("Configuración", configWidget);

    // ----- Metrics content (grid de cards) -----
    auto *metricsGridWidget = new QWidget(central);
    auto *grid = new QGridLayout(metricsGridWidget);
    grid->setContentsMargins(0,0,0,0);
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(12);

    // Uptime card
    {
        auto *w = new QWidget(metricsGridWidget);
        auto *l = new QVBoxLayout(w);
        l->setContentsMargins(0,0,0,0);
        l->setSpacing(6);

        auto *label = new QLabel("Uptime", w);
        label->setStyleSheet(QString("color:%1;").arg(COLOR_MUTED));
        m_lblUptime = new QLabel("—", w);
        QFont vf = m_lblUptime->font();
        vf.setPointSize(vf.pointSize() + 4);
        vf.setBold(true);
        m_lblUptime->setFont(vf);
        m_lblUptime->setStyleSheet(QString("color:%1;").arg(COLOR_TEXT));

        l->addWidget(label);
        l->addWidget(m_lblUptime);
        l->addStretch(1);

        grid->addWidget(createCard("Uptime", w), 0, 0);
    }

    // Load card
    {
        auto *w = new QWidget(metricsGridWidget);
        auto *l = new QVBoxLayout(w);
        l->setContentsMargins(0,0,0,0);
        l->setSpacing(6);

        auto *label = new QLabel("Load (1/5/15)", w);
        label->setStyleSheet(QString("color:%1;").arg(COLOR_MUTED));

        m_lblLoad = new QLabel("—", w);
        QFont vf = m_lblLoad->font();
        vf.setPointSize(vf.pointSize() + 4);
        vf.setBold(true);
        m_lblLoad->setFont(vf);
        m_lblLoad->setStyleSheet(QString("color:%1;").arg(COLOR_TEXT));

        l->addWidget(label);
        l->addWidget(m_lblLoad);
        l->addStretch(1);

        grid->addWidget(createCard("Load", w), 0, 1);
    }

    // RAM card (progress)
    {
        auto *w = new QWidget(metricsGridWidget);
        auto *l = new QVBoxLayout(w);
        l->setContentsMargins(0,0,0,0);
        l->setSpacing(8);

        auto *label = new QLabel("Memoria", w);
        label->setStyleSheet(QString("color:%1;").arg(COLOR_MUTED));

        m_lblMemText = new QLabel("—", w);
        m_lblMemText->setStyleSheet(QString("color:%1;").arg(COLOR_TEXT));

        m_pbMem = new QProgressBar(w);
        m_pbMem->setRange(0, 100);
        m_pbMem->setValue(0);
        m_pbMem->setTextVisible(true);
        m_pbMem->setFormat("%p%");

        l->addWidget(label);
        l->addWidget(m_lblMemText);
        l->addWidget(m_pbMem);
        l->addStretch(1);

        grid->addWidget(createCard("RAM", w), 1, 0);
    }

    // Disk card (progress)
    {
        auto *w = new QWidget(metricsGridWidget);
        auto *l = new QVBoxLayout(w);
        l->setContentsMargins(0,0,0,0);
        l->setSpacing(8);

        auto *label = new QLabel("Disco", w);
        label->setStyleSheet(QString("color:%1;").arg(COLOR_MUTED));

        m_lblDiskText = new QLabel("—", w);
        m_lblDiskText->setStyleSheet(QString("color:%1;").arg(COLOR_TEXT));

        m_pbDisk = new QProgressBar(w);
        m_pbDisk->setRange(0, 100);
        m_pbDisk->setValue(0);
        m_pbDisk->setTextVisible(true);
        m_pbDisk->setFormat("%p%");

        l->addWidget(label);
        l->addWidget(m_lblDiskText);
        l->addWidget(m_pbDisk);
        l->addStretch(1);

        grid->addWidget(createCard("Disk", w), 1, 1);
    }

    // Wrap metrics area in card
    auto *metricsCard = createCard("Métricas principales", metricsGridWidget);

    topRow->addWidget(configCard, 1);
    topRow->addWidget(metricsCard, 2);
    root->addLayout(topRow);

    // ===== Historial =====
    auto *eventsContent = new QWidget(central);
    auto *eventsLayout = new QVBoxLayout(eventsContent);
    eventsLayout->setContentsMargins(0,0,0,0);
    eventsLayout->setSpacing(8);

    m_txtEvents = new QPlainTextEdit(eventsContent);
    m_txtEvents->setReadOnly(true);
    m_txtEvents->setPlaceholderText("Eventos (cambios de estado) ...");
    eventsLayout->addWidget(m_txtEvents);

    root->addWidget(createCard("Historial", eventsContent));

    // ===== Conexiones =====
    connect(m_btnRefresh, &QPushButton::clicked, this, &MainWindow::onRefreshClicked);
    connect(m_sbInterval, &QSpinBox::valueChanged, this, &MainWindow::onIntervalChanged);

    connect(&m_service, &MonitorService::metricsUpdated, this, &MainWindow::onMetricsUpdated);
    connect(&m_service, &MonitorService::requestStateChanged, this, &MainWindow::onRequestStateChanged);

    // ===== Config / start =====
    loadSettings();
    m_service.setEndpointUrl(QUrl(m_leUrl->text()));
    m_service.setIntervalSeconds(m_sbInterval->value());
    m_service.setRequestTimeoutMs(3000);

    appendEvent("Iniciando monitoreo...");
    m_service.start();

    setStatus(PanelState::Down, "Sin datos todavía");
}

void MainWindow::applyGrafanaDarkTheme() {
    // Palette base
    QPalette p;
    p.setColor(QPalette::Window, QColor(COLOR_BG));
    p.setColor(QPalette::WindowText, QColor(COLOR_TEXT));
    p.setColor(QPalette::Base, QColor("#0f1730"));
    p.setColor(QPalette::AlternateBase, QColor("#0f1730"));
    p.setColor(QPalette::ToolTipBase, QColor(COLOR_TEXT));
    p.setColor(QPalette::ToolTipText, QColor(COLOR_BG));
    p.setColor(QPalette::Text, QColor(COLOR_TEXT));
    p.setColor(QPalette::Button, QColor("#16213a"));
    p.setColor(QPalette::ButtonText, QColor(COLOR_TEXT));
    p.setColor(QPalette::BrightText, Qt::red);
    p.setColor(QPalette::Link, QColor("#4aa3ff"));
    p.setColor(QPalette::Highlight, QColor("#2b67f6"));
    p.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    qApp->setPalette(p);

    // Global stylesheet (cards, inputs, groupboxes, progress)
    qApp->setStyleSheet(QString(R"(
        QWidget { color: %1; }
        QGroupBox {
            border: 1px solid %2;
            border-radius: 10px;
            margin-top: 10px;
            padding: 10px;
            background: %3;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 6px 0 6px;
            color: %4;
        }
        QLineEdit, QSpinBox, QDoubleSpinBox, QPlainTextEdit {
            background: #0f1730;
            border: 1px solid %2;
            border-radius: 8px;
            padding: 6px;
            selection-background-color: #2b67f6;
        }
        QPushButton {
            background: #2b67f6;
            border: 0px;
            border-radius: 8px;
            padding: 8px 10px;
            font-weight: 600;
        }
        QPushButton:hover { background: #3a74ff; }
        QPushButton:disabled { background: #2b3550; color: %4; }

        QProgressBar {
            border: 1px solid %2;
            border-radius: 8px;
            text-align: center;
            background: #0f1730;
            height: 18px;
        }
        QProgressBar::chunk {
            border-radius: 8px;
            background: #2ecc71;
        }
    )").arg(COLOR_TEXT, COLOR_BORDER, COLOR_PANEL, COLOR_MUTED));
}

QWidget* MainWindow::createCard(const QString &title, QWidget *content) {
    auto *box = new QGroupBox(title, this);
    auto *l = new QVBoxLayout(box);
    l->setContentsMargins(12, 12, 12, 12);
    l->addWidget(content);
    return box;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    saveSettings();
    QMainWindow::closeEvent(event);
}

void MainWindow::loadSettings() {
    QSettings s("UTN", "VpsMonitorPanel");

    m_leUrl->setText(s.value("endpoint/url", m_leUrl->text()).toString());
    m_sbInterval->setValue(s.value("endpoint/interval_seconds", m_sbInterval->value()).toInt());

    m_sbLoadWarn->setValue(s.value("thresholds/load1_warn", m_sbLoadWarn->value()).toDouble());
    m_sbMemWarn->setValue(s.value("thresholds/mem_warn_percent", m_sbMemWarn->value()).toInt());
    m_sbDiskWarn->setValue(s.value("thresholds/disk_warn_percent", m_sbDiskWarn->value()).toInt());

    appendEvent("Configuración cargada");
}

void MainWindow::saveSettings() const {
    QSettings s("UTN", "VpsMonitorPanel");

    s.setValue("endpoint/url", m_leUrl->text());
    s.setValue("endpoint/interval_seconds", m_sbInterval->value());

    s.setValue("thresholds/load1_warn", m_sbLoadWarn->value());
    s.setValue("thresholds/mem_warn_percent", m_sbMemWarn->value());
    s.setValue("thresholds/disk_warn_percent", m_sbDiskWarn->value());
}

void MainWindow::onRefreshClicked() {
    m_service.setEndpointUrl(QUrl(m_leUrl->text()));
    appendEvent("Refresco manual");
    m_service.refreshNow();
}

void MainWindow::onIntervalChanged(int v) {
    m_service.setIntervalSeconds(v);
    appendEvent(QString("Intervalo cambiado a %1 s").arg(v));
}

MainWindow::PanelState MainWindow::computeState(const VpsMetrics &m, QString *reasonOut) const {
    if (!m.ok) {
        if (reasonOut) *reasonOut = m.error;
        return PanelState::Down;
    }

    QStringList reasons;

    const double loadWarn = m_sbLoadWarn->value();
    const int memWarn = m_sbMemWarn->value();
    const int diskWarn = m_sbDiskWarn->value();

    if (m.load1 >= loadWarn)
        reasons << QString("Load 1m %1 >= %2").arg(m.load1, 0, 'f', 2).arg(loadWarn, 0, 'f', 2);

    if (m.memUsedPercent >= memWarn)
        reasons << QString("RAM %1% >= %2%").arg(m.memUsedPercent, 0, 'f', 1).arg(memWarn);

    if (m.diskUsedPercent >= diskWarn)
        reasons << QString("Disco %1% >= %2%").arg(m.diskUsedPercent, 0, 'f', 1).arg(diskWarn);

    if (reasonOut) *reasonOut = reasons.join(" | ");

    return reasons.isEmpty() ? PanelState::Ok : PanelState::Alert;
}

void MainWindow::setStatus(PanelState st, const QString &reason) {
    QString badgeText;
    QString badgeColor;

    switch (st) {
    case PanelState::Ok:
        badgeText = "OK";
        badgeColor = COLOR_OK;
        break;
    case PanelState::Alert:
        badgeText = "ALERTA";
        badgeColor = COLOR_WARN;
        break;
    case PanelState::Down:
        badgeText = "CAÍDO";
        badgeColor = COLOR_DOWN;
        break;
    }

    m_lblStatusBadge->setText(badgeText);
    m_lblStatusBadge->setStyleSheet(QString(
                                        "background:%1; color:#0b1220; border-radius:10px; padding:6px 10px;")
                                        .arg(badgeColor));

    m_lblStatusDetail->setText(reason);
}

void MainWindow::onMetricsUpdated(const VpsMetrics &m) {
    m_lblLastCheck->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));

    if (!m.ok) {
        setStatus(PanelState::Down, "Sin respuesta: " + m.error);

        if (m_lastState != PanelState::Down) {
            appendEvent("Estado: CAÍDO (" + m.error + ")");
            m_lastState = PanelState::Down;
            m_lastAlertReason.clear();
        }
        return;
    }

    // Textos grandes
    m_lblUptime->setText(QString::number(m.uptimeSeconds) + " s");
    m_lblLoad->setText(QString("%1 / %2 / %3").arg(m.load1).arg(m.load5).arg(m.load15));

    // RAM
    m_lblMemText->setText(QString("%1/%2 MB (%3%)")
                              .arg(m.memUsedMb)
                              .arg(m.memTotalMb)
                              .arg(m.memUsedPercent, 0, 'f', 1));
    m_pbMem->setValue(qBound(0, (int)qRound(m.memUsedPercent), 100));

    // Disk
    m_lblDiskText->setText(QString("%1: %2/%3 GB (%4%)")
                               .arg(m.diskMount)
                               .arg(m.diskUsedGb, 0, 'f', 2)
                               .arg(m.diskTotalGb, 0, 'f', 2)
                               .arg(m.diskUsedPercent, 0, 'f', 1));
    m_pbDisk->setValue(qBound(0, (int)qRound(m.diskUsedPercent), 100));

    // Estado + razón
    QString reason;
    const PanelState st = computeState(m, &reason);
    setStatus(st, (st == PanelState::Ok) ? "Dentro de umbrales" : reason);

    // Progresos coloreados según umbral
    auto setChunkColor = [](QProgressBar *pb, const QString &color) {
        pb->setStyleSheet(QString("QProgressBar::chunk { background: %1; }").arg(color));
    };

    const int memWarn = m_sbMemWarn->value();
    const int diskWarn = m_sbDiskWarn->value();

    setChunkColor(m_pbMem, (m.memUsedPercent >= memWarn) ? COLOR_WARN : COLOR_OK);
    setChunkColor(m_pbDisk, (m.diskUsedPercent >= diskWarn) ? COLOR_WARN : COLOR_OK);

    // Historial por transiciones (y cambio de razón)
    if (st != m_lastState || (st == PanelState::Alert && reason != m_lastAlertReason)) {
        if (st == PanelState::Ok) {
            appendEvent("Estado: OK");
        } else if (st == PanelState::Alert) {
            appendEvent("Estado: ALERTA (" + reason + ")");
        } else {
            appendEvent("Estado: CAÍDO");
        }

        m_lastState = st;
        m_lastAlertReason = reason;
    }
}

void MainWindow::onRequestStateChanged(bool inFlight) {
    m_btnRefresh->setEnabled(!inFlight);
}

void MainWindow::appendEvent(const QString &line) {
    QStringList lines = m_txtEvents->toPlainText().split('\n', Qt::SkipEmptyParts);
    lines.append(QDateTime::currentDateTime().toString("HH:mm:ss") + " - " + line);

    const int maxLines = 25;
    while (lines.size() > maxLines) lines.removeFirst();

    m_txtEvents->setPlainText(lines.join('\n'));
    m_txtEvents->verticalScrollBar()->setValue(m_txtEvents->verticalScrollBar()->maximum());
}