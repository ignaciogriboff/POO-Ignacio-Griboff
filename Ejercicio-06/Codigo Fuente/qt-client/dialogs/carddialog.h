#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QString>

/**
 * CardDialog — Dialog for creating or editing a card.
 */
class CardDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CardDialog(QWidget *parent = nullptr,
                        const QString &currentTitle = {},
                        const QString &currentDesc  = {});
    QString title() const;
    QString description() const;

private:
    QLineEdit *m_titleEdit = nullptr;
    QTextEdit *m_descEdit  = nullptr;
};
