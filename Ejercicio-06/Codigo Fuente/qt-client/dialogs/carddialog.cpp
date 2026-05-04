#include "carddialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QMessageBox>

CardDialog::CardDialog(QWidget *parent, const QString &currentTitle, const QString &currentDesc)
    : QDialog(parent)
{
    setWindowTitle(currentTitle.isEmpty() ? tr("New Card") : tr("Edit Card"));
    setMinimumWidth(360);

    auto *lay = new QVBoxLayout(this);

    lay->addWidget(new QLabel(tr("Card title:"), this));
    m_titleEdit = new QLineEdit(this);
    m_titleEdit->setPlaceholderText(tr("e.g. Fix login bug"));
    m_titleEdit->setText(currentTitle);
    lay->addWidget(m_titleEdit);

    lay->addWidget(new QLabel(tr("Description (optional):"), this));
    m_descEdit = new QTextEdit(this);
    m_descEdit->setPlaceholderText(tr("Details..."));
    m_descEdit->setPlainText(currentDesc);
    m_descEdit->setFixedHeight(100);
    lay->addWidget(m_descEdit);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    lay->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        if (m_titleEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, tr("Validation"), tr("Title must not be empty."));
            return;
        }
        accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QString CardDialog::title() const
{
    return m_titleEdit->text().trimmed();
}

QString CardDialog::description() const
{
    return m_descEdit->toPlainText().trimmed();
}
