#pragma once

#include <QWidget>
#include <QColor>
#include <QPainterPath>
#include <QTimer>

#include "drawingmodel.h"

class CanvasWidget : public QWidget {
    Q_OBJECT
public:
    explicit CanvasWidget(QWidget* parent = nullptr);

    QSize minimumSizeHint() const override { return {640, 480}; }

    const DrawingModel& model() const { return m_model; }
    DrawingModel& model() { return m_model; }

signals:
    void strokeStarted(const Stroke& strokeMeta);
    void strokePointsChunk(const QString& strokeId, const QVector<QPointF>& points);
    void strokeEnded(const QString& strokeId);

protected:
    void paintEvent(QPaintEvent* e) override;

    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

    void wheelEvent(QWheelEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;

private:
    void addInterpolatedPoints(const QPointF& from, const QPointF& to);
    static QPainterPath buildSmoothPath(const QVector<StrokePoint>& pts);
    static QColor lerpColor(const QColor& a, const QColor& b, double t);

    void flushPendingPoints();

private:
    DrawingModel m_model;

    bool m_drawing = false;
    bool m_eraser = false;

    QColor m_currentColor;
    qreal m_currentWidth = 6.0;

    QPointF m_lastPoint;

    // streaming
    QTimer m_flushTimer;
    QVector<QPointF> m_pendingPoints;
};