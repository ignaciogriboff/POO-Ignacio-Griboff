#pragma once
#include <QDialog>
#include "tpitem.h"

class QLineEdit;
class QComboBox;
class QPushButton;

class TpEditDialog : public QDialog {
    Q_OBJECT
public:
    // Para ALTA
    explicit TpEditDialog(QWidget* parent = nullptr);

    // Para EDICIÓN (carga valores)
    explicit TpEditDialog(const TpItem& existing, QWidget* parent = nullptr);

    TpItem resultTp() const;

private:
    void buildUi();
    void setFromTp(const TpItem& tp);

    static int statusToIndex(TpStatus s);
    static TpStatus indexToStatus(int idx);

    static int prioToIndex(TpPriority p);
    static TpPriority indexToPrio(int idx);

private:
    int m_id = 0;

    QLineEdit* m_titleEdit = nullptr;
    QComboBox* m_statusCombo = nullptr;
    QComboBox* m_prioCombo = nullptr;
    QPushButton* m_saveBtn = nullptr;
    QPushButton* m_cancelBtn = nullptr;
};