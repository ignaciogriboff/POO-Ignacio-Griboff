#pragma once

#include "database.h"

#include <QColor>
#include <QList>
#include <QWidget>

class Pintura : public QWidget {
    Q_OBJECT

public:
    explicit Pintura(int userId, QWidget *parent = nullptr);

    int brushSize() const;
    QColor brushColor() const;

signals:
    void statusMessageChanged(const QString &message);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void drawStroke(QPainter &painter, const StrokeData &stroke) const;
    void loadPersistedStrokes();
    void updateStatus();
    void setBrushColor(const QColor &color, const QString &name);
    void clearCanvasAndPersistence();
    void undoLastStroke();

    int m_userId;
    QList<StrokeData> m_strokes;
    StrokeData m_currentStroke;
    bool m_drawing = false;

    QColor m_brushColor = Qt::black;
    int m_brushSize = 4;

    QList<int> m_undoStrokeIds;
    static constexpr int kMaxUndo = 10;
};
