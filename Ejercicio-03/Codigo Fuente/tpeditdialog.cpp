#include "tpeditdialog.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

TpEditDialog::TpEditDialog(QWidget* parent)
    : QDialog(parent) {
    buildUi();
    setWindowTitle("Nuevo trabajo práctico");
}

TpEditDialog::TpEditDialog(const TpItem& existing, QWidget* parent)
    : QDialog(parent),
    m_id(existing.id) {
    buildUi();
    setWindowTitle("Editar trabajo práctico");
    setFromTp(existing);
}

void TpEditDialog::buildUi() {
    auto* root = new QVBoxLayout();

    auto* titleLbl = new QLabel("Título:");
    m_titleEdit = new QLineEdit();

    auto* statusLbl = new QLabel("Estado:");
    m_statusCombo = new QComboBox();
    m_statusCombo->addItem("Pendiente");
    m_statusCombo->addItem("En progreso");
    m_statusCombo->addItem("Entregado");

    auto* prioLbl = new QLabel("Prioridad:");
    m_prioCombo = new QComboBox();
    m_prioCombo->addItem("Baja");
    m_prioCombo->addItem("Media");
    m_prioCombo->addItem("Alta");

    auto* btnRow = new QHBoxLayout();
    btnRow->addStretch();

    m_saveBtn = new QPushButton("Guardar");
    m_cancelBtn = new QPushButton("Cancelar");
    btnRow->addWidget(m_saveBtn);
    btnRow->addWidget(m_cancelBtn);

    root->addWidget(titleLbl);
    root->addWidget(m_titleEdit);
    root->addSpacing(8);
    root->addWidget(statusLbl);
    root->addWidget(m_statusCombo);
    root->addSpacing(8);
    root->addWidget(prioLbl);
    root->addWidget(m_prioCombo);
    root->addSpacing(12);
    root->addLayout(btnRow);

    setLayout(root);
    setMinimumWidth(420);

    connect(m_cancelBtn, &QPushButton::clicked, this, &TpEditDialog::reject);
    connect(m_saveBtn, &QPushButton::clicked, this, [this]() {
        if (m_titleEdit->text().trimmed().isEmpty()) {
            m_titleEdit->setFocus();
            return; // simple validación
        }
        accept();
    });
}

void TpEditDialog::setFromTp(const TpItem& tp) {
    m_titleEdit->setText(tp.titulo);
    m_statusCombo->setCurrentIndex(statusToIndex(tp.estado));
    m_prioCombo->setCurrentIndex(prioToIndex(tp.prioridad));
}

TpItem TpEditDialog::resultTp() const {
    TpItem tp;
    tp.id = m_id; // si es alta, esto queda 0 y lo asigna MainWindowWidget
    tp.titulo = m_titleEdit->text().trimmed();
    tp.estado = indexToStatus(m_statusCombo->currentIndex());
    tp.prioridad = indexToPrio(m_prioCombo->currentIndex());
    return tp;
}

int TpEditDialog::statusToIndex(TpStatus s) {
    switch (s) {
    case TpStatus::Pendiente:  return 0;
    case TpStatus::EnProgreso: return 1;
    case TpStatus::Entregado:  return 2;
    }
    return 0;
}

TpStatus TpEditDialog::indexToStatus(int idx) {
    switch (idx) {
    case 0: return TpStatus::Pendiente;
    case 1: return TpStatus::EnProgreso;
    case 2: return TpStatus::Entregado;
    }
    return TpStatus::Pendiente;
}

int TpEditDialog::prioToIndex(TpPriority p) {
    switch (p) {
    case TpPriority::Baja:  return 0;
    case TpPriority::Media: return 1;
    case TpPriority::Alta:  return 2;
    }
    return 1;
}

TpPriority TpEditDialog::indexToPrio(int idx) {
    switch (idx) {
    case 0: return TpPriority::Baja;
    case 1: return TpPriority::Media;
    case 2: return TpPriority::Alta;
    }
    return TpPriority::Media;
}