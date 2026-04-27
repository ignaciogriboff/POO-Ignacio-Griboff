#include "mainwindow.h"
#include "canvaswidget.h"
#include "syncservice.h"
#include "realtimesyncservice.h"

#include <QToolBar>
#include <QAction>
#include <QMessageBox>
#include <QStatusBar>
#include <QUrl>

static bool modelContainsStrokeId(const DrawingModel& model, const QString& id) {
    if (id.isEmpty()) return false;
    for (const auto& s : model.strokes()) {
        if (s.id == id) return true;
    }
    return false;
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Lienzo colaborativo (Paso 3.1 - Streaming WS)");
    resize(1000, 700);

    m_canvas = new CanvasWidget(this);
    setCentralWidget(m_canvas);

    // HTTP (guardar/cargar)
    m_sync = new SyncService(QUrl("http://173.212.234.190:8082"), this);

    // WS (tiempo real)
    m_rt = new RealtimeSyncService(QUrl("ws://173.212.234.190:8082/ws"), this);

    auto* tb = addToolBar("Toolbar");
    tb->setMovable(false);
    QAction* actSave = tb->addAction("Guardar");

    tb->setStyleSheet(R"(
        QToolBar { background: #f2f2f2; spacing: 8px; padding: 6px; }
        QToolButton {
            background: #0078d4; color: white;
            padding: 8px 14px; border-radius: 4px;
            font-weight: 600;
        }
        QToolButton:hover { background: #106ebe; }
        QToolButton:pressed { background: #005a9e; }
    )");

    // Estado WS
    connect(m_rt, &RealtimeSyncService::connectionStateChanged, this, [this](const QString& s) {
        statusBar()->showMessage(s, 2500);
    });

    // 1) Cargar por HTTP al iniciar
    statusBar()->showMessage("Cargando canvas (HTTP)...");
    connect(m_sync, &SyncService::loadFinished, this, [this](bool ok, const QString& msg) {
        statusBar()->showMessage(msg, 4000);
        if (!ok) QMessageBox::warning(this, "Carga desde VPS", msg);

        m_canvas->update();

        // 2) Conectar WS luego de cargar (para evitar duplicación con snapshot WS)
        m_rt->connectToServer();
    });
    m_sync->loadCanvas(&m_canvas->model());

    // 3) Guardar manual por HTTP
    connect(m_sync, &SyncService::saveFinished, this, [this](bool ok, const QString& msg) {
        statusBar()->showMessage(msg, 4000);
        if (!ok) QMessageBox::warning(this, "Guardar en VPS", msg);
    });
    connect(actSave, &QAction::triggered, this, [this]() {
        statusBar()->showMessage("Guardando en VPS (HTTP)...");
        m_sync->saveCanvas(&m_canvas->model());
    });

    // 4) Local -> WS (streaming)
    connect(m_canvas, &CanvasWidget::strokeStarted, this, [this](const Stroke& s) {
        m_rt->sendStrokeBegin(s);
    });
    connect(m_canvas, &CanvasWidget::strokePointsChunk, this, [this](const QString& id, const QVector<QPointF>& pts) {
        m_rt->sendStrokePoints(id, pts);
    });
    connect(m_canvas, &CanvasWidget::strokeEnded, this, [this](const QString& id) {
        m_rt->sendStrokeEnd(id);
    });

    // 5) Remoto -> merge en vivo
    connect(m_rt, &RealtimeSyncService::remoteStrokeBegin, this, [this](const Stroke& s) {
        if (modelContainsStrokeId(m_canvas->model(), s.id)) return;

        int idx = m_canvas->model().strokes().size();
        m_canvas->model().strokes().push_back(s); // points vacío
        m_remoteInProgressIndex.insert(s.id, idx);
        m_canvas->update();
    });

    connect(m_rt, &RealtimeSyncService::remoteStrokePoints, this, [this](const QString& id, const QVector<QPointF>& pts) {
        if (!m_remoteInProgressIndex.contains(id)) return;

        int idx = m_remoteInProgressIndex.value(id);
        if (idx < 0 || idx >= m_canvas->model().strokes().size()) return;

        Stroke& st = m_canvas->model().strokes()[idx];
        for (const auto& p : pts) {
            st.points.push_back(StrokePoint{p});
        }
        m_canvas->update();
    });

    connect(m_rt, &RealtimeSyncService::remoteStrokeEnd, this, [this](const QString& id) {
        m_remoteInProgressIndex.remove(id);
    });
}