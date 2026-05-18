#include "pajaro.h"

#include <QPainter>

Pajaro::Pajaro(TipoPajaro tipo, QWidget *parent)
    : QWidget(parent)
    , m_timer(new QTimer(this))
    , m_globalSpeed(6.0)
    , m_typeBonus(0.0)
    , m_tipo(tipo)
    , m_frame(0)
{
    switch (m_tipo) {
    case Pequeno:
        setFixedSize(30, 18);
        m_typeBonus = 1.2;
        break;
    case Mediano:
        setFixedSize(42, 22);
        m_typeBonus = 0.9;
        break;
    case Grande:
        setFixedSize(56, 28);
        m_typeBonus = 0.6;
        break;
    }

    connect(m_timer, SIGNAL(timeout()), this, SLOT(onMoveTimeout()));
    m_timer->start(30);
}

void Pajaro::setGlobalSpeed(qreal speed)
{
    m_globalSpeed = speed;
}

void Pajaro::stop()
{
    m_timer->stop();
}

void Pajaro::onMoveTimeout()
{
    ++m_frame;
    const int delta = qMax(2, qRound(m_globalSpeed + m_typeBonus));
    move(x() - delta, y());

    if (x() + width() < 0) {
        emit leftScreen(this);
    }
}

void Pajaro::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QColor bodyColor;
    QColor wingColor;
    switch (m_tipo) {
    case Pequeno:
        bodyColor = QColor(220, 120, 0);
        wingColor = QColor(255, 178, 74);
        break;
    case Mediano:
        bodyColor = QColor(190, 80, 10);
        wingColor = QColor(224, 123, 44);
        break;
    case Grande:
        bodyColor = QColor(160, 40, 0);
        wingColor = QColor(208, 73, 36);
        break;
    }

    const int flap = (m_frame % 18 < 9) ? -2 : 2;

    painter.setPen(QPen(QColor(112, 50, 15), 1));
    painter.setBrush(bodyColor);
    painter.drawRoundedRect(QRect(6, 3, width() - 12, height() - 6), 8, 8);

    painter.setBrush(wingColor);
    painter.drawEllipse(QRect(2, 0, width() / 2, height() - 2).translated(0, flap));

    painter.setBrush(QColor(245, 196, 88));
    painter.setPen(Qt::NoPen);
    painter.drawPolygon(QPolygonF({
        QPointF(width() - 1, height() / 2),
        QPointF(width() - 7, height() / 2 - 3),
        QPointF(width() - 7, height() / 2 + 3)
    }));

    painter.setBrush(Qt::white);
    painter.drawEllipse(width() - 12, 5, 4, 4);
    painter.setBrush(Qt::black);
    painter.drawEllipse(width() - 11, 6, 2, 2);

    painter.setPen(QPen(QColor(112, 50, 15), 1));
    painter.drawLine(9, height() - 3, 12, height() + 2);
    painter.drawLine(16, height() - 3, 19, height() + 2);
}
