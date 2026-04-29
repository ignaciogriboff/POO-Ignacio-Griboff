#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include <QTimer>
#include <QPixmap>

#include "clima_service.h"
#include "image_cache_service.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Login; }
QT_END_NAMESPACE

class Login : public QWidget
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = nullptr);
    ~Login();

    void setStatusText(const QString &text);

signals:
    void loginOk();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onLoginClicked();
    void onLockTick();

private:
    void setLocked(bool locked);
    void showStatus(const QString &text, const QString &color = "white");
    void applyBackground(const QPixmap &pm);

    Ui::Login *ui = nullptr;

    int m_failedAttempts = 0;
    bool m_locked = false;

    QTimer m_lockTimer;
    int m_lockRemainingSec = 0;

    static constexpr int kMaxAttempts = 3;
    static constexpr int kLockSeconds = 10;

    // Clima
    ClimaService m_clima;

    // Hora
    QTimer m_clockTimer;

    // Fondo con cache
    ImageCacheService m_img;
    QPixmap m_bgOriginal;
};

#endif // LOGIN_H