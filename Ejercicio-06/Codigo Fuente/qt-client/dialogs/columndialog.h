#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QString>

/**
 * ColumnDialog — Dialog for creating or renaming a column.
 */
class ColumnDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ColumnDialog(QWidget *parent = nullptr, const QString &currentTitle = {});
    QString title() const;

private:
    QLineEdit *m_titleEdit = nullptr;
};
