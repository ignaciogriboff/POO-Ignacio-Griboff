#include "mainwindowwidget.h"
#include "tpeditdialog.h"
#include "notesstore.h"
#include "notesdialog.h"
#include "historylogger.h"

#include <QComboBox>
#include <QDir>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

MainWindowWidget::MainWindowWidget(const QString& username, QWidget* parent)
    : QWidget(parent),
    m_username(username) {
    setWindowTitle(QString("Planificador TP - %1").arg(m_username));
    resize(950, 620);

    auto* root = new QVBoxLayout();

    auto* header = new QLabel(QString("Tablero de trabajos prácticos (Usuario: %1)").arg(m_username));
    QFont hf = header->font();
    hf.setPointSize(12);
    hf.setBold(true);
    header->setFont(hf);

    auto* topRow = new QHBoxLayout();

    m_newBtn = new QPushButton("Nuevo TP");
    topRow->addWidget(m_newBtn);

    topRow->addSpacing(18);

    auto* statusLbl = new QLabel("Estado:");
    m_statusFilter = new QComboBox();
    m_statusFilter->addItem("Todos");
    m_statusFilter->addItem("Pendiente");
    m_statusFilter->addItem("En progreso");
    m_statusFilter->addItem("Entregado");

    auto* prioLbl = new QLabel("Prioridad:");
    m_priorityFilter = new QComboBox();
    m_priorityFilter->addItem("Todas");
    m_priorityFilter->addItem("Baja");
    m_priorityFilter->addItem("Media");
    m_priorityFilter->addItem("Alta");

    topRow->addWidget(statusLbl);
    topRow->addWidget(m_statusFilter);
    topRow->addSpacing(12);
    topRow->addWidget(prioLbl);
    topRow->addWidget(m_priorityFilter);
    topRow->addStretch();

    m_scroll = new QScrollArea();
    m_scroll->setWidgetResizable(true);

    m_gridContainer = new QWidget();
    m_grid = new QGridLayout();
    m_grid->setColumnStretch(0, 2);
    m_grid->setColumnStretch(1, 1);
    m_grid->setColumnStretch(2, 1);
    m_grid->setColumnStretch(3, 1);
    m_gridContainer->setLayout(m_grid);

    m_scroll->setWidget(m_gridContainer);

    // Historial
    auto* historyTitle = new QLabel("Historial de acciones:");
    QFont htF = historyTitle->font();
    htF.setBold(true);
    historyTitle->setFont(htF);

    m_historyList = new QListWidget();
    m_historyList->setMinimumHeight(140);

    root->addWidget(header);
    root->addLayout(topRow);
    root->addSpacing(8);
    root->addWidget(m_scroll);
    root->addSpacing(10);
    root->addWidget(historyTitle);
    root->addWidget(m_historyList);

    setLayout(root);

    connect(m_newBtn, &QPushButton::clicked, this, &MainWindowWidget::onNewClicked);
    connect(m_statusFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindowWidget::onFilterChanged);
    connect(m_priorityFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindowWidget::onFilterChanged);

    // Stores / logger
    m_notesStore = new NotesStore(QDir::current().absoluteFilePath("notes.json"));
    m_history = new HistoryLogger(QDir::current().absoluteFilePath("history.log"));

    // --- NUEVO: cargar historial existente del archivo ---
    loadHistoryOnStartup();

    // Registrar evento de apertura (opcional)
    appendHistory("APP_START", "Se abrió la ventana principal");

    seedMockData();
    refreshGrid();
}

MainWindowWidget::~MainWindowWidget() {
    delete m_notesStore;
    m_notesStore = nullptr;

    delete m_history;
    m_history = nullptr;
}

void MainWindowWidget::loadHistoryOnStartup() {
    if (!m_history || !m_historyList) return;

    m_historyList->clear();
    const QStringList lines = m_history->readAllLines();
    for (const QString& line : lines) {
        m_historyList->addItem(line);
    }
    if (!lines.isEmpty())
        m_historyList->scrollToBottom();
}

void MainWindowWidget::appendHistory(const QString& action, const QString& details) {
    if (!m_history || !m_historyList) return;
    const QString line = m_history->log(m_username, action, details);
    m_historyList->addItem(line);
    m_historyList->scrollToBottom();
}

// (El resto del archivo queda igual que tu versión del punto 6)

void MainWindowWidget::seedMockData() {
    m_items.clear();
    m_nextId = 1;

    auto add = [this](const QString& t, TpStatus s, TpPriority p) {
        TpItem item;
        item.id = m_nextId++;
        item.titulo = t;
        item.estado = s;
        item.prioridad = p;
        m_items.push_back(item);
    };

    add("TP1 - Intro a Qt Widgets", TpStatus::Pendiente,  TpPriority::Alta);
    add("TP2 - Señales y slots",     TpStatus::EnProgreso, TpPriority::Media);
    add("TP3 - JSON local",          TpStatus::Entregado,  TpPriority::Baja);
    add("TP4 - UI con grilla",       TpStatus::Pendiente,  TpPriority::Media);
    add("TP5 - Notas del TP",        TpStatus::EnProgreso, TpPriority::Alta);
}

void MainWindowWidget::onFilterChanged() {
    refreshGrid();
    appendHistory("FILTRO", QString("Estado=%1 | Prioridad=%2")
                                .arg(m_statusFilter->currentText(),
                                     m_priorityFilter->currentText()));
}

void MainWindowWidget::onNewClicked() {
    addTp();
}

bool MainWindowWidget::matchesFilter(const TpItem& tp) const {
    const int sIdx = m_statusFilter->currentIndex();
    if (sIdx != 0) {
        if (statusToString(tp.estado) != m_statusFilter->currentText()) return false;
    }

    const int pIdx = m_priorityFilter->currentIndex();
    if (pIdx != 0) {
        if (priorityToString(tp.prioridad) != m_priorityFilter->currentText()) return false;
    }

    return true;
}

int MainWindowWidget::indexOfId(int id) const {
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == id) return i;
    }
    return -1;
}

void MainWindowWidget::clearGrid() {
    while (QLayoutItem* item = m_grid->takeAt(0)) {
        if (QWidget* w = item->widget()) w->deleteLater();
        delete item;
    }
}

void MainWindowWidget::refreshGrid() {
    clearGrid();

    int row = 0;

    // Header columnas
    auto* hTitulo = new QLabel("Título");
    auto* hEstado = new QLabel("Estado");
    auto* hPrio   = new QLabel("Prioridad");
    auto* hAcc    = new QLabel("Acciones");
    QFont bold; bold.setBold(true);
    hTitulo->setFont(bold);
    hEstado->setFont(bold);
    hPrio->setFont(bold);
    hAcc->setFont(bold);

    m_grid->addWidget(hTitulo, row, 0);
    m_grid->addWidget(hEstado, row, 1);
    m_grid->addWidget(hPrio,   row, 2);
    m_grid->addWidget(hAcc,    row, 3);
    row++;

    int shown = 0;

    for (const TpItem& tp : m_items) {
        if (!matchesFilter(tp)) continue;

        auto* title = new QLabel(tp.titulo);
        auto* estado = new QLabel(statusToString(tp.estado));
        auto* prio = new QLabel(priorityToString(tp.prioridad));

        auto* actionsWidget = new QWidget();
        auto* actionsLayout = new QHBoxLayout();
        actionsLayout->setContentsMargins(0, 0, 0, 0);

        auto* editBtn = new QPushButton("Ver/Editar");
        auto* notesBtn = new QPushButton("Notas");
        auto* delBtn = new QPushButton("Eliminar");

        actionsLayout->addWidget(editBtn);
        actionsLayout->addWidget(notesBtn);
        actionsLayout->addWidget(delBtn);
        actionsWidget->setLayout(actionsLayout);

        connect(editBtn, &QPushButton::clicked, this, [this, tp]() { editTp(tp.id); });
        connect(delBtn, &QPushButton::clicked, this, [this, tp]() { deleteTp(tp.id); });

        // Notas (Punto 5) + log (Punto 6)
        connect(notesBtn, &QPushButton::clicked, this, [this, tp]() {
            NotesDialog dlg(tp.id, tp.titulo, m_notesStore, this);

            connect(&dlg, &NotesDialog::noteSaved, this, [this, tp](int) {
                appendHistory("GUARDAR_NOTA",
                              QString("TP#%1 \"%2\"").arg(tp.id).arg(tp.titulo));
            });

            dlg.exec();
        });

        m_grid->addWidget(title, row, 0);
        m_grid->addWidget(estado, row, 1);
        m_grid->addWidget(prio, row, 2);
        m_grid->addWidget(actionsWidget, row, 3);

        row++;
        shown++;
    }

    if (shown == 0) {
        auto* empty = new QLabel("No hay trabajos prácticos para el filtro seleccionado.");
        empty->setStyleSheet("color:#aaaaaa;");
        m_grid->addWidget(empty, row, 0, 1, 4);
        row++;
    }

    m_grid->setRowStretch(row + 1, 1);
}

void MainWindowWidget::addTp() {
    TpEditDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;

    TpItem tp = dlg.resultTp();
    tp.id = m_nextId++;
    m_items.push_back(tp);

    appendHistory("ALTA_TP", QString("TP#%1 \"%2\"").arg(tp.id).arg(tp.titulo));
    refreshGrid();
}

void MainWindowWidget::editTp(int id) {
    const int idx = indexOfId(id);
    if (idx < 0) return;

    TpEditDialog dlg(m_items[idx], this);
    if (dlg.exec() != QDialog::Accepted) return;

    TpItem updated = dlg.resultTp();
    updated.id = id;

    const QString beforeTitle = m_items[idx].titulo;
    m_items[idx] = updated;

    appendHistory("EDIT_TP", QString("TP#%1 \"%2\" (antes: \"%3\")")
                                 .arg(id)
                                 .arg(updated.titulo)
                                 .arg(beforeTitle));
    refreshGrid();
}

void MainWindowWidget::deleteTp(int id) {
    const int idx = indexOfId(id);
    if (idx < 0) return;

    const QString title = m_items[idx].titulo;

    const auto reply = QMessageBox::question(
        this,
        "Confirmar eliminación",
        QString("¿Eliminar el TP \"%1\"?").arg(title),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    m_items.removeAt(idx);

    appendHistory("DEL_TP", QString("TP#%1 \"%2\"").arg(id).arg(title));
    refreshGrid();
}