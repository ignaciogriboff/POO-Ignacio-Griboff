#pragma once

#include <QColor>
#include <QPointF>
#include <QVector>
#include <QString>
#include <QJsonObject>

struct StrokePoint {
    QPointF p;
};

struct Stroke {
    QString id;
    QString author;
    QVector<StrokePoint> points;
    QColor color;
    qreal width = 3.0;
    bool eraser = false;   // true => goma (pinta blanco)
};

class DrawingModel {
public:
    void clear();

    void beginStroke(const QString& author, const QColor& color, qreal width, bool eraser);
    void addPoint(const QPointF& p);
    void endStroke();

    const QVector<Stroke>& strokes() const { return m_strokes; }
    QVector<Stroke>& strokes() { return m_strokes; }

    // JSON (guardar/cargar por HTTP)
    QJsonObject toJson() const;
    bool fromJson(const QJsonObject& obj, QString* error = nullptr);

private:
    QVector<Stroke> m_strokes;
    bool m_inStroke = false;
};