#include "trex.h"

#include <QPainter>

TRex::TRex(QWidget *parent)
    : QWidget(parent)
    , m_groundY(0)
    , m_verticalVelocity(0)
    , m_isJumping(false)
    , m_isCrouching(false)
    , m_normalWidth(46)
    , m_normalHeight(54)
    , m_crouchHeight(34)
{
    setFixedSize(m_normalWidth, m_normalHeight);
}

void TRex::setGroundY(int groundY)
{
    m_groundY = groundY;
    move(x(), m_groundY - height());
}

void TRex::resetPose()
{
    m_verticalVelocity = 0;
    m_isJumping = false;
    m_isCrouching = false;
    setFixedSize(m_normalWidth, m_normalHeight);
    move(90, m_groundY - height());
    update();
}

void TRex::jump()
{
    if (m_isJumping) {
        return;
    }

    m_isJumping = true;
    m_verticalVelocity = -17;
}

void TRex::crouch(bool enabled)
{
    if (m_isJumping) {
        return;
    }

    m_isCrouching = enabled;
    const int oldBottom = y() + height();

    if (m_isCrouching) {
        setFixedSize(m_normalWidth + 10, m_crouchHeight);
    } else {
        setFixedSize(m_normalWidth, m_normalHeight);
    }

    move(x(), oldBottom - height());
    update();
}

void TRex::moveForward()
{
    const int maxX = parentWidget() ? parentWidget()->width() / 2 : x() + 25;
    move(qMin(x() + 22, maxX), y());
}

void TRex::moveBackward()
{
    move(qMax(25, x() - 22), y());
}

void TRex::updatePhysics()
{
    if (!m_isJumping) {
        return;
    }

    m_verticalVelocity += 1;
    move(x(), y() + m_verticalVelocity);

    const int floorY = m_groundY - height();
    if (y() >= floorY) {
        move(x(), floorY);
        m_verticalVelocity = 0;
        m_isJumping = false;
    }
}

QRect TRex::hitBox() const
{
    QRect box = geometry();
    box.adjust(5, 5, -5, -4);
    return box;
}

void TRex::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setPen(QPen(QColor(28, 68, 30), 2));

    const QRect bodyRect(6, 14, width() - 16, height() - 20);
    const QRect headRect(width() - 22, 5, 18, 18);
    const QRect tailRect(0, height() - 18, 18, 8);
    const QRect legBack(10, height() - 15, 9, 13);
    const QRect legFront(width() - 20, height() - 15, 9, 13);

    painter.setBrush(QColor(53, 129, 60));
    painter.drawRoundedRect(bodyRect, 10, 10);
    painter.drawRoundedRect(headRect, 7, 7);
    painter.drawRoundedRect(tailRect, 4, 4);
    painter.drawRoundedRect(legBack, 3, 3);
    painter.drawRoundedRect(legFront, 3, 3);

    painter.setBrush(QColor(76, 160, 79));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(bodyRect.adjusted(6, 5, -8, -8));
    painter.drawEllipse(headRect.adjusted(3, 3, -3, -4));

    painter.setPen(QPen(QColor(36, 84, 37), 2));
    painter.drawLine(bodyRect.left() + 10, bodyRect.bottom() - 5, bodyRect.left() + 2, bodyRect.bottom() + 2);
    painter.drawLine(bodyRect.left() + 4, bodyRect.bottom() - 8, bodyRect.left() - 6, bodyRect.bottom() - 2);
    painter.drawLine(bodyRect.right() - 8, bodyRect.bottom() - 6, bodyRect.right() + 3, bodyRect.bottom() - 2);

    painter.setBrush(Qt::white);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(width() - 15, 10, 5, 5);

    painter.setBrush(Qt::black);
    painter.drawEllipse(width() - 13, 11, 2, 2);
}
