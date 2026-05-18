#ifndef TREX_H
#define TREX_H

#include <QWidget>

class TRex : public QWidget
{
    Q_OBJECT

public:
    explicit TRex(QWidget *parent = nullptr);

    void setGroundY(int groundY);
    void resetPose();
    void jump();
    void crouch(bool enabled);
    void moveForward();
    void moveBackward();
    void updatePhysics();
    QRect hitBox() const;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int m_groundY;
    int m_verticalVelocity;
    bool m_isJumping;
    bool m_isCrouching;
    int m_normalWidth;
    int m_normalHeight;
    int m_crouchHeight;
};

#endif // TREX_H
