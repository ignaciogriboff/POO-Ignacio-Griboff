#ifndef PAJARO_H
#define PAJARO_H

#include <QTimer>
#include <QWidget>

class Pajaro : public QWidget
{
    Q_OBJECT

public:
    enum TipoPajaro {
        Pequeno,
        Mediano,
        Grande
    };

    explicit Pajaro(TipoPajaro tipo, QWidget *parent = nullptr);

    void setGlobalSpeed(qreal speed);
    void stop();

signals:
    void leftScreen(Pajaro *pajaro);

private slots:
    void onMoveTimeout();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QTimer *m_timer;
    qreal m_globalSpeed;
    qreal m_typeBonus;
    TipoPajaro m_tipo;
    int m_frame;
};

#endif // PAJARO_H
