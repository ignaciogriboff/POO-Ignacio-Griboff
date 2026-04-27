#pragma once
#include <QWidget>
#include <QVector>

#include "tpitem.h"

class QComboBox;
class QGridLayout;
class QScrollArea;
class QWidget;
class QPushButton;
class QListWidget;

class NotesStore;
class HistoryLogger;

class MainWindowWidget : public QWidget {
    Q_OBJECT
public:
    explicit MainWindowWidget(const QString& username, QWidget* parent = nullptr);
    ~MainWindowWidget();

private slots:
    void onFilterChanged();
    void onNewClicked();

private:
    void seedMockData();
    void refreshGrid();
    void clearGrid();

    bool matchesFilter(const TpItem& tp) const;
    int indexOfId(int id) const;

    void addTp();
    void editTp(int id);
    void deleteTp(int id);

    void appendHistory(const QString& action, const QString& details);
    void loadHistoryOnStartup(); // <-- NUEVO

private:
    QString m_username;

    QPushButton* m_newBtn = nullptr;

    QComboBox* m_statusFilter = nullptr;
    QComboBox* m_priorityFilter = nullptr;

    QScrollArea* m_scroll = nullptr;
    QWidget* m_gridContainer = nullptr;
    QGridLayout* m_grid = nullptr;

    QVector<TpItem> m_items;
    int m_nextId = 1;

    NotesStore* m_notesStore = nullptr;

    HistoryLogger* m_history = nullptr;
    QListWidget* m_historyList = nullptr;
};