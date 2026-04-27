#include "notesdialog.h"
#include "notesstore.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

NotesDialog::NotesDialog(int tpId,
                         const QString& tpTitle,
                         NotesStore* store,
                         QWidget* parent)
    : QDialog(parent),
    m_tpId(tpId),
    m_store(store),
    m_tpTitle(tpTitle) {
    setWindowTitle(QString("Notas - TP #%1").arg(m_tpId));
    setMinimumSize(520, 360);

    auto* root = new QVBoxLayout();

    auto* title = new QLabel(QString("Notas para: %1 (ID %2)").arg(m_tpTitle).arg(m_tpId));
    QFont f = title->font();
    f.setBold(true);
    title->setFont(f);

    m_textEdit = new QTextEdit();

    auto* btnRow = new QHBoxLayout();
    btnRow->addStretch();

    m_saveBtn = new QPushButton("Guardar");
    m_closeBtn = new QPushButton("Cerrar");

    btnRow->addWidget(m_saveBtn);
    btnRow->addWidget(m_closeBtn);

    root->addWidget(title);
    root->addWidget(m_textEdit);
    root->addLayout(btnRow);

    setLayout(root);

    connect(m_saveBtn, &QPushButton::clicked, this, &NotesDialog::onSaveClicked);
    connect(m_closeBtn, &QPushButton::clicked, this, &NotesDialog::reject);

    // Cargar nota existente
    if (m_store) {
        const QString existing = m_store->loadNote(m_tpId);
        m_textEdit->setPlainText(existing);
    }
}

void NotesDialog::onSaveClicked() {
    if (!m_store) return;

    const QString text = m_textEdit->toPlainText();
    const bool ok = m_store->saveNote(m_tpId, text);

    if (!ok) {
        QMessageBox::warning(this, "Error", "No se pudo guardar la nota (notes.json).");
        return;
    }

    emit noteSaved(m_tpId); // <-- NUEVO

    QMessageBox::information(this, "OK", "Nota guardada.");
}