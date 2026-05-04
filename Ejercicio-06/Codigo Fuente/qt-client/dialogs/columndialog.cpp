#include "columndialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QMessageBox>

ColumnDialog::ColumnDialog(QWidget *parent, const QString &currentTitle)
    : QDialog(parent)
{
    setWindowTitle(currentTitle.isEmpty() ? tr("New Column") : tr("Edit Column"));
    setMinimumWidth(300);

    auto *lay = new QVBoxLayout(this);

    lay->addWidget(new QLabel(tr("Column title:"), this));
    m_titleEdit = new QLineEdit(this);
    m_titleEdit->setPlaceholderText(tr("e.g. To Do"));
    m_titleEdit->setText(currentTitle);
    lay->addWidget(m_titleEdit);

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

QString ColumnDialog::title() const
{
    return m_titleEdit->text().trimmed();
}
