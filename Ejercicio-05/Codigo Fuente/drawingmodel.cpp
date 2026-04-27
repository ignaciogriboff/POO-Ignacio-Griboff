#include "drawingmodel.h"

#include <QJsonArray>
#include <QUuid>

void DrawingModel::clear() {
    m_strokes.clear();
    m_inStroke = false;
}

void DrawingModel::beginStroke(const QString& author, const QColor& color, qreal width, bool eraser) {
    Stroke s;
    s.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    s.author = author;
    s.color = color;
    s.width = width;
    s.eraser = eraser;
    m_strokes.push_back(std::move(s));
    m_inStroke = true;
}

void DrawingModel::addPoint(const QPointF& p) {
    if (!m_inStroke || m_strokes.isEmpty()) return;
    m_strokes.last().points.push_back(StrokePoint{p});
}

void DrawingModel::endStroke() {
    m_inStroke = false;
}

QJsonObject DrawingModel::toJson() const {
    QJsonObject root;
    root["version"] = 1;

    QJsonArray strokesArr;

    for (const Stroke& s : m_strokes) {
        QJsonObject so;
        so["id"] = s.id;
        if (!s.author.isEmpty()) so["author"] = s.author;

        so["eraser"] = s.eraser;
        so["width"] = s.width;

        QJsonObject col;
        col["r"] = s.color.red();
        col["g"] = s.color.green();
        col["b"] = s.color.blue();
        so["color"] = col;

        QJsonArray pts;
        for (const auto& sp : s.points) {
            QJsonObject po;
            po["x"] = sp.p.x();
            po["y"] = sp.p.y();
            pts.append(po);
        }
        so["points"] = pts;

        strokesArr.append(so);
    }

    root["strokes"] = strokesArr;
    return root;
}

bool DrawingModel::fromJson(const QJsonObject& obj, QString* error) {
    auto fail = [&](const QString& msg) {
        if (error) *error = msg;
        return false;
    };

    if (!obj.contains("strokes") || !obj["strokes"].isArray())
        return fail("JSON inválido: falta 'strokes' (array).");

    QJsonArray strokesArr = obj["strokes"].toArray();

    QVector<Stroke> loaded;
    loaded.reserve(strokesArr.size());

    for (const auto& v : strokesArr) {
        if (!v.isObject()) return fail("JSON inválido: stroke no es objeto.");
        QJsonObject so = v.toObject();

        Stroke s;
        s.id = so.value("id").toString();
        s.author = so.value("author").toString();
        s.eraser = so.value("eraser").toBool(false);
        s.width = so.value("width").toDouble(3.0);

        if (!so.contains("color") || !so["color"].isObject())
            return fail("JSON inválido: falta 'color'.");

        QJsonObject col = so["color"].toObject();
        s.color = QColor(col.value("r").toInt(0), col.value("g").toInt(0), col.value("b").toInt(0));

        if (!so.contains("points") || !so["points"].isArray())
            return fail("JSON inválido: falta 'points'.");

        QJsonArray pts = so["points"].toArray();
        s.points.reserve(pts.size());
        for (const auto& pv : pts) {
            if (!pv.isObject()) return fail("JSON inválido: point no es objeto.");
            QJsonObject po = pv.toObject();
            s.points.push_back(StrokePoint{QPointF(po.value("x").toDouble(), po.value("y").toDouble())});
        }

        loaded.push_back(std::move(s));
    }

    m_strokes = std::move(loaded);
    m_inStroke = false;
    return true;
}