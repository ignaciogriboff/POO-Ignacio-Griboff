#pragma once
#include <QDialog>

class QTextEdit;
class QPushButton;
class NotesStore;

class NotesDialog : public QDialog {
    Q_OBJECT
public:
    explicit NotesDialog(int tpId,
                         const QString& tpTitle,
                         NotesStore* store,
                         QWidget* parent = nullptr);

signals:
    void noteSaved(int tpId); // <-- NUEVO

private slots:
    void onSaveClicked();

private:
    int m_tpId = 0;
    NotesStore* m_store = nullptr;

    QTextEdit* m_textEdit = nullptr;
    QPushButton* m_saveBtn = nullptr;
    QPushButton* m_closeBtn = nullptr;

    QString m_tpTitle;
};