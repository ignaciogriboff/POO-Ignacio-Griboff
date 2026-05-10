#include "pintura.h"

#include "database.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QWheelEvent>

Pintura::Pintura(int userId, QWidget *parent)
    : QWidget(parent)
    , m_userId(userId)
{
    setObjectName("paintCanvas");
    setAttribute(Qt::WA_StaticContents);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setMinimumSize(800, 500);

    loadPersistedStrokes();
    updateStatus();
}

int Pintura::brushSize() const
{
    return m_brushSize;
}

QColor Pintura::brushColor() const
{
    return m_brushColor;
}

void Pintura::loadPersistedStrokes()
{
    QString errorMessage;
    m_strokes = Database::loadStrokesForUser(m_userId, &errorMessage);

    if (!errorMessage.isEmpty()) {
        emit statusMessageChanged(QString("Error al cargar trazos: %1").arg(errorMessage));
    }
}

void Pintura::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    const QRect canvasRect = rect();
    painter.fillRect(canvasRect, Qt::white);

    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(QPen(QColor(216, 236, 224), 1));
    constexpr int gridStep = 24;
    for (int x = 0; x < width(); x += gridStep) {
        painter.drawLine(x, 0, x, height());
    }
    for (int y = 0; y < height(); y += gridStep) {
        painter.drawLine(0, y, width(), y);
    }

    painter.setRenderHint(QPainter::Antialiasing, true);

    for (const StrokeData &stroke : m_strokes) {
        drawStroke(painter, stroke);
    }

    if (m_drawing && !m_currentStroke.points.isEmpty()) {
        drawStroke(painter, m_currentStroke);
    }
}

void Pintura::drawStroke(QPainter &painter, const StrokeData &stroke) const
{
    if (stroke.points.isEmpty()) {
        return;
    }

    QPen pen(stroke.color, stroke.thickness, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);

    if (stroke.points.size() == 1) {
        painter.drawPoint(stroke.points.first());
        return;
    }

    for (int i = 1; i < stroke.points.size(); ++i) {
        painter.drawLine(stroke.points.at(i - 1), stroke.points.at(i));
    }
}

void Pintura::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        return;
    }

    setFocus();
    m_drawing = true;
    m_currentStroke = StrokeData();
    m_currentStroke.color = m_brushColor;
    m_currentStroke.thickness = m_brushSize;
    m_currentStroke.points.append(event->position().toPoint());

    update();
}

void Pintura::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_drawing || !(event->buttons() & Qt::LeftButton)) {
        return;
    }

    m_currentStroke.points.append(event->position().toPoint());
    update();
}

void Pintura::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_drawing || event->button() != Qt::LeftButton) {
        return;
    }

    m_currentStroke.points.append(event->position().toPoint());
    m_drawing = false;

    if (m_currentStroke.points.size() < 2) {
        update();
        return;
    }

    QString errorMessage;
    const int strokeId = Database::saveStrokeForUser(m_userId, m_currentStroke, &errorMessage);
    if (strokeId < 0) {
        emit statusMessageChanged(QString("No se pudo guardar el trazo: %1").arg(errorMessage));
        update();
        return;
    }

    m_currentStroke.id = strokeId;
    m_strokes.append(m_currentStroke);

    m_undoStrokeIds.append(strokeId);
    while (m_undoStrokeIds.size() > kMaxUndo) {
        m_undoStrokeIds.removeFirst();
    }

    m_currentStroke = StrokeData();
    updateStatus();
    update();
}

void Pintura::wheelEvent(QWheelEvent *event)
{
    const int degrees = event->angleDelta().y() / 8;
    const int steps = degrees / 15;

    if (steps == 0) {
        return;
    }

    m_brushSize = qBound(1, m_brushSize + steps, 30);
    updateStatus();
    event->accept();
}

void Pintura::setBrushColor(const QColor &color, const QString &name)
{
    m_brushColor = color;
    emit statusMessageChanged(QString("Color actual: %1 | Grosor: %2").arg(name).arg(m_brushSize));
}

void Pintura::clearCanvasAndPersistence()
{
    QString errorMessage;
    if (!Database::clearStrokesForUser(m_userId, &errorMessage)) {
        emit statusMessageChanged(QString("No se pudo limpiar el lienzo: %1").arg(errorMessage));
        return;
    }

    m_strokes.clear();
    m_currentStroke = StrokeData();
    m_undoStrokeIds.clear();
    emit statusMessageChanged("Lienzo borrado (Escape)");
    update();
}

void Pintura::undoLastStroke()
{
    if (m_undoStrokeIds.isEmpty()) {
        emit statusMessageChanged("No hay acciones para deshacer");
        return;
    }

    const int strokeId = m_undoStrokeIds.takeLast();

    QString errorMessage;
    if (!Database::deleteStrokeById(strokeId, &errorMessage)) {
        emit statusMessageChanged(QString("Error al deshacer: %1").arg(errorMessage));
        return;
    }

    for (int i = m_strokes.size() - 1; i >= 0; --i) {
        if (m_strokes.at(i).id == strokeId) {
            m_strokes.removeAt(i);
            break;
        }
    }

    emit statusMessageChanged("Deshacer aplicado (Ctrl+Z)");
    update();
}

void Pintura::keyPressEvent(QKeyEvent *event)
{
    if (event->matches(QKeySequence::Undo)) {
        undoLastStroke();
        return;
    }

    if (event->key() == Qt::Key_Escape) {
        clearCanvasAndPersistence();
        return;
    }

    if (event->key() == Qt::Key_R) {
        setBrushColor(QColor(255, 91, 91), "Rojo");
        return;
    }

    if (event->key() == Qt::Key_G) {
        setBrushColor(QColor(75, 255, 159), "Verde");
        return;
    }

    if (event->key() == Qt::Key_B) {
        setBrushColor(QColor(84, 158, 255), "Azul");
        return;
    }

    QWidget::keyPressEvent(event);
}

void Pintura::updateStatus()
{
    QString colorName = "Negro";
    if (m_brushColor == QColor(255, 91, 91)) {
        colorName = "Rojo";
    } else if (m_brushColor == QColor(75, 255, 159)) {
        colorName = "Verde";
    } else if (m_brushColor == QColor(84, 158, 255)) {
        colorName = "Azul";
    }

    emit statusMessageChanged(QString("Color actual: %1 | Grosor: %2 | Undo disponibles: %3/%4")
                                  .arg(colorName)
                                  .arg(m_brushSize)
                                  .arg(m_undoStrokeIds.size())
                                  .arg(kMaxUndo));
}
