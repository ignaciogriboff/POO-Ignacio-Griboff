#pragma once

#include <QMainWindow>
#include <QHash>
#include <QString>

class CanvasWidget;
class SyncService;
class RealtimeSyncService;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    CanvasWidget* m_canvas = nullptr;
    SyncService* m_sync = nullptr;
    RealtimeSyncService* m_rt = nullptr;

    // strokeId -> index en m_canvas->model().strokes()
    QHash<QString, int> m_remoteInProgressIndex;
};