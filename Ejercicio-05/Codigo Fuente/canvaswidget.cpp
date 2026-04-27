#include "canvaswidget.h"

#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QLineF>
#include <algorithm>
#include <cmath>

static const QColor kColorStart(192, 19, 76);
static const QColor kColorEnd(24, 233, 199);

CanvasWidget::CanvasWidget(QWidget* parent)
    : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    m_currentColor = kColorStart;

    // Fondo blanco
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::white);
    setPalette(pal);

    // streaming flush timer (~40fps)
    m_flushTimer.setInterval(25);
    connect(&m_flushTimer, &QTimer::timeout, this, &CanvasWidget::flushPendingPoints);
}

QColor CanvasWidget::lerpColor(const QColor& a, const QColor& b, double t) {
    t = std::clamp(t, 0.0, 1.0);
    int r  = int(a.red()   + (b.red()   - a.red())   * t);
    int g  = int(a.green() + (b.green() - a.green()) * t);
    int bl = int(a.blue()  + (b.blue()  - a.blue())  * t);
    return QColor(r, g, bl);
}

QPainterPath CanvasWidget::buildSmoothPath(const QVector<StrokePoint>& pts) {
    QPainterPath path;
    if (pts.isEmpty()) return path;

    path.moveTo(pts[0].p);

    if (pts.size() == 1) {
        path.lineTo(pts[0].p + QPointF(0.01, 0.01));
        return path;
    }

    QPointF prev = pts[0].p;
    for (int i = 1; i < pts.size(); ++i) {
        QPointF cur = pts[i].p;
        QPointF mid = (prev + cur) * 0.5;
        path.quadTo(prev, mid);
        prev = cur;
    }
    path.quadTo(prev, prev);
    return path;
}

void CanvasWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    p.fillRect(rect(), Qt::white);

    for (const Stroke& s : m_model.strokes()) {
        if (s.points.isEmpty()) continue;

        QPainterPath path = buildSmoothPath(s.points);

        QPen pen;
        pen.setWidthF(s.width);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);

        pen.setColor(s.eraser ? Qt::white : s.color);

        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        p.setPen(pen);
        p.drawPath(path);
    }
}

void CanvasWidget::flushPendingPoints() {
    if (!m_drawing) return;
    if (m_model.strokes().isEmpty()) return;
    if (m_pendingPoints.isEmpty()) return;

    const QString strokeId = m_model.strokes().last().id;
    emit strokePointsChunk(strokeId, m_pendingPoints);
    m_pendingPoints.clear();
}

void CanvasWidget::addInterpolatedPoints(const QPointF& from, const QPointF& to) {
    const double step = 2.0;

    double dist = QLineF(from, to).length();
    if (dist <= step) {
        m_model.addPoint(to);
        m_pendingPoints.push_back(to);
        return;
    }

    int n = int(std::ceil(dist / step));
    for (int i = 1; i <= n; ++i) {
        double t = double(i) / double(n);
        QPointF pi = from + (to - from) * t;
        m_model.addPoint(pi);
        m_pendingPoints.push_back(pi);
    }
}

void CanvasWidget::mousePressEvent(QMouseEvent* e) {
    if (e->button() != Qt::LeftButton && e->button() != Qt::RightButton)
        return;

    m_drawing = true;
    m_eraser = (e->button() == Qt::RightButton);

    m_model.beginStroke("ignaciogriboff", m_currentColor, m_currentWidth, m_eraser);

    m_lastPoint = e->position();
    m_model.addPoint(m_lastPoint);

    // streaming begin (sin puntos)
    emit strokeStarted(m_model.strokes().last());

    // puntos acumulados para enviar
    m_pendingPoints.clear();
    m_pendingPoints.push_back(m_lastPoint);

    m_flushTimer.start();
    update();
}

void CanvasWidget::mouseMoveEvent(QMouseEvent* e) {
    if (!m_drawing) return;

    QPointF cur = e->position();
    addInterpolatedPoints(m_lastPoint, cur);
    m_lastPoint = cur;

    update();
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent* e) {
    if (!m_drawing) return;

    if (e->button() == Qt::LeftButton || e->button() == Qt::RightButton) {
        // enviar últimos puntos
        flushPendingPoints();

        m_drawing = false;
        m_flushTimer.stop();

        QString id;
        if (!m_model.strokes().isEmpty())
            id = m_model.strokes().last().id;

        m_model.endStroke();

        if (!id.isEmpty())
            emit strokeEnded(id);

        update();
    }
}

void CanvasWidget::wheelEvent(QWheelEvent* e) {
    const QPoint numDegrees = e->angleDelta() / 8;
    const QPoint numSteps = numDegrees / 15;

    if (!numSteps.isNull()) {
        m_currentWidth += numSteps.y();
        m_currentWidth = std::clamp(m_currentWidth, 1.0, 60.0);
        update();
    }
    e->accept();
}

void CanvasWidget::keyPressEvent(QKeyEvent* e) {
    int key = e->key();
    if (key >= Qt::Key_1 && key <= Qt::Key_9) {
        int n = key - Qt::Key_1;    // 0..8
        double t = double(n) / 8.0; // 0..1
        m_currentColor = lerpColor(kColorStart, kColorEnd, t);
        update();
        return;
    }

    QWidget::keyPressEvent(e);
}