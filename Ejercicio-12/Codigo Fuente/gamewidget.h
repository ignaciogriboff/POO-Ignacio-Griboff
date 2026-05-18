#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QImage>
#include <QList>
#include <QTimer>
#include <QWidget>

class TRex;
class Pajaro;

class GameWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GameWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    void onGameTick();
    void onSpawnBird();
    void onBirdLeftScreen(Pajaro *pajaro);
    void onSurpriseObstacle();

private:
    struct Cactus {
        QRect rect;
    };

    void spawnCactus(bool surprise = false);
    void removeOffscreenCactus();
    void checkCollisions();
    void triggerGameOver();
    void restartGame();
    void clearBirds();
    QImage buildGameOverImage() const;

    TRex *m_trex;
    QTimer *m_gameTimer;
    QTimer *m_birdSpawnTimer;

    QList<Cactus> m_cactus;
    QList<Pajaro *> m_birds;

    qreal m_baseSpeed;
    int m_elapsedMs;
    int m_score;
    bool m_isGameOver;

    QImage m_gameOverImage;
};

#endif // GAMEWIDGET_H
