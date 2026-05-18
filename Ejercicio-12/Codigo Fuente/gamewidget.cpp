#include "gamewidget.h"

#include "pajaro.h"
#include "trex.h"

#include <QKeyEvent>
#include <QPainter>
#include <QRandomGenerator>

GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent)
    , m_trex(new TRex(this))
    , m_gameTimer(new QTimer(this))
    , m_birdSpawnTimer(new QTimer(this))
    , m_baseSpeed(6.0)
    , m_elapsedMs(0)
    , m_score(0)
    , m_isGameOver(false)
    , m_gameOverImage(buildGameOverImage())
{
    setWindowTitle("Ejercicio 10 - T-Rex Extremo");
    setFixedSize(960, 320);
    setFocusPolicy(Qt::StrongFocus);

    const int groundY = height() - 36;
    m_trex->move(90, groundY - m_trex->height());
    m_trex->setGroundY(groundY);

    connect(m_gameTimer, SIGNAL(timeout()), this, SLOT(onGameTick()));
    connect(m_birdSpawnTimer, SIGNAL(timeout()), this, SLOT(onSpawnBird()));

    m_gameTimer->start(16);
    m_birdSpawnTimer->start(5000);

    spawnCactus();
    spawnCactus();

    QTimer::singleShot(7000, this, SLOT(onSurpriseObstacle()));
}

void GameWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QLinearGradient sky(0, 0, 0, height());
    sky.setColorAt(0.0, QColor(255, 244, 221));
    sky.setColorAt(1.0, QColor(241, 223, 176));
    painter.fillRect(rect(), sky);

    const int groundY = height() - 36;
    painter.setPen(QPen(QColor(75, 75, 75), 2));
    painter.drawLine(0, groundY, width(), groundY);

    painter.setPen(Qt::NoPen);
    for (const Cactus &c : m_cactus) {
        const QRect trunk = c.rect.adjusted(4, 0, -4, 0);
        const QRect armLeft(trunk.left() - trunk.width() / 2, trunk.top() + trunk.height() / 3,
                            trunk.width() / 2, trunk.height() / 3);
        const QRect armRight(trunk.right() - trunk.width() / 4, trunk.top() + trunk.height() / 2,
                             trunk.width() / 2, trunk.height() / 4);

        painter.setBrush(QColor(49, 139, 54));
        painter.drawRoundedRect(trunk, 5, 5);
        painter.drawRoundedRect(armLeft, 4, 4);
        painter.drawRoundedRect(armRight, 4, 4);

        painter.setPen(QPen(QColor(31, 78, 32), 2));
        painter.drawLine(trunk.center().x(), trunk.top() + 5, trunk.center().x(), trunk.bottom() - 4);
        painter.drawLine(armLeft.center().x(), armLeft.top() + 2, armLeft.center().x(), armLeft.bottom() - 2);
        painter.drawLine(armRight.center().x(), armRight.top() + 2, armRight.center().x(), armRight.bottom() - 2);

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(227, 182, 85));
        painter.drawEllipse(trunk.right() - 5, trunk.top() + 2, 5, 5);
    }

    painter.setPen(QColor(40, 40, 40));
    QFont scoreFont("Consolas", 12, QFont::Bold);
    painter.setFont(scoreFont);
    painter.drawText(20, 28, QString("Score: %1").arg(m_score));

    if (m_isGameOver) {
        painter.fillRect(rect(), QColor(0, 0, 0, 110));

        const int x = (width() - m_gameOverImage.width()) / 2;
        const int y = (height() - m_gameOverImage.height()) / 2 - 20;
        painter.drawImage(x, y, m_gameOverImage);

        painter.setPen(Qt::white);
        painter.setFont(QFont("Consolas", 12, QFont::DemiBold));
        painter.drawText(0, y + m_gameOverImage.height() + 36, width(), 20,
                         Qt::AlignHCenter, "Presiona R para reiniciar");
    }
}

void GameWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) {
        return;
    }

    if (m_isGameOver) {
        if (event->key() == Qt::Key_R) {
            restartGame();
        }
        return;
    }

    switch (event->key()) {
    case Qt::Key_Space:
    	case Qt::Key_Up:
        m_trex->jump();
        break;
    case Qt::Key_Down:
        m_trex->crouch(true);
        break;
    case Qt::Key_Right:
        m_trex->moveForward();
        break;
    case Qt::Key_Left:
        m_trex->moveBackward();
        break;
    default:
        QWidget::keyPressEvent(event);
        break;
    }
}

void GameWidget::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) {
        return;
    }

    if (event->key() == Qt::Key_Down) {
        m_trex->crouch(false);
        return;
    }

    QWidget::keyReleaseEvent(event);
}

void GameWidget::onGameTick()
{
    if (m_isGameOver) {
        return;
    }

    m_score += 1;
    m_elapsedMs += m_gameTimer->interval();

    if (m_elapsedMs % 1500 < m_gameTimer->interval()) {
        if (QRandomGenerator::global()->bounded(100) < 45) {
            spawnCactus();
        }
    }

    if (m_elapsedMs % 7000 < m_gameTimer->interval()) {
        m_baseSpeed += 0.45;
    }

    for (Cactus &c : m_cactus) {
        c.rect.translate(-qRound(m_baseSpeed), 0);
    }

    removeOffscreenCactus();
    m_trex->updatePhysics();

    for (Pajaro *bird : m_birds) {
        bird->setGlobalSpeed(m_baseSpeed);
    }

    checkCollisions();
    update();
}

void GameWidget::onSpawnBird()
{
    if (m_isGameOver) {
        return;
    }

    const int randomType = QRandomGenerator::global()->bounded(3);
    Pajaro::TipoPajaro type = Pajaro::Mediano;
    if (randomType == 0) {
        type = Pajaro::Pequeno;
    } else if (randomType == 2) {
        type = Pajaro::Grande;
    }

    Pajaro *bird = new Pajaro(type, this);
    bird->setGlobalSpeed(m_baseSpeed);

    const int heights[] = {
        height() - 150,
        height() - 125,
        height() - 100
    };
    const int y = heights[QRandomGenerator::global()->bounded(3)];
    const int x = width() + QRandomGenerator::global()->bounded(40, 180);

    bird->move(x, y);
    connect(bird, SIGNAL(leftScreen(Pajaro*)), this, SLOT(onBirdLeftScreen(Pajaro*)));
    bird->show();

    m_birds.append(bird);
}

void GameWidget::onBirdLeftScreen(Pajaro *pajaro)
{
    if (!pajaro) {
        return;
    }

    m_birds.removeAll(pajaro);
    pajaro->deleteLater();
}

void GameWidget::onSurpriseObstacle()
{
    if (m_isGameOver) {
        return;
    }

    spawnCactus(true);

    if (QRandomGenerator::global()->bounded(100) < 35) {
        onSpawnBird();
    }

    const int nextMs = QRandomGenerator::global()->bounded(7000, 12000);
    QTimer::singleShot(nextMs, this, SLOT(onSurpriseObstacle()));
}

void GameWidget::spawnCactus(bool surprise)
{
    Cactus c;
    const int baseHeight = surprise
        ? QRandomGenerator::global()->bounded(40, 65)
        : QRandomGenerator::global()->bounded(32, 58);
    const int cactusWidth = QRandomGenerator::global()->bounded(18, 30);

    const int groundY = height() - 36;
    const int startX = this->width() + QRandomGenerator::global()->bounded(0, 220);
    c.rect = QRect(startX, groundY - baseHeight, cactusWidth, baseHeight);

    m_cactus.append(c);
}

void GameWidget::removeOffscreenCactus()
{
    for (int i = m_cactus.size() - 1; i >= 0; --i) {
        if (m_cactus[i].rect.right() < 0) {
            m_cactus.removeAt(i);
        }
    }
}

void GameWidget::checkCollisions()
{
    const QRect dinoHit = m_trex->hitBox();

    for (const Cactus &c : m_cactus) {
        if (dinoHit.intersects(c.rect.adjusted(2, 2, -2, -2))) {
            triggerGameOver();
            return;
        }
    }

    for (Pajaro *bird : m_birds) {
        if (dinoHit.intersects(bird->geometry().adjusted(2, 2, -2, -2))) {
            triggerGameOver();
            return;
        }
    }
}

void GameWidget::triggerGameOver()
{
    m_isGameOver = true;
    m_gameTimer->stop();
    m_birdSpawnTimer->stop();

    for (Pajaro *bird : m_birds) {
        bird->stop();
    }

    update();
}

void GameWidget::restartGame()
{
    m_isGameOver = false;
    m_elapsedMs = 0;
    m_score = 0;
    m_baseSpeed = 6.0;

    m_cactus.clear();
    clearBirds();

    m_trex->resetPose();

    spawnCactus();
    spawnCactus();

    m_gameTimer->start(16);
    m_birdSpawnTimer->start(5000);

    QTimer::singleShot(7000, this, SLOT(onSurpriseObstacle()));
    update();
}

void GameWidget::clearBirds()
{
    while (!m_birds.isEmpty()) {
        Pajaro *bird = m_birds.takeLast();
        bird->stop();
        bird->deleteLater();
    }
}

QImage GameWidget::buildGameOverImage() const
{
    QImage image(420, 120, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter p(&image);
    p.setRenderHint(QPainter::Antialiasing, true);

    p.setBrush(QColor(214, 60, 35));
    p.setPen(QPen(QColor(250, 236, 230), 3));
    p.drawRoundedRect(image.rect().adjusted(2, 2, -3, -3), 14, 14);

    p.setPen(Qt::white);
    p.setFont(QFont("Consolas", 30, QFont::Black));
    p.drawText(image.rect(), Qt::AlignCenter, "GAME OVER");

    return image;
}
