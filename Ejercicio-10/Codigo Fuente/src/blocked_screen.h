#pragma once

#include "pantalla.h"

class QLabel;
class QProgressBar;
class QTimer;

class BlockedScreen final : public Pantalla {
    Q_OBJECT

public:
    explicit BlockedScreen(int lockSeconds, QWidget* parent = nullptr);

    void inicializarUI() override;
    void conectarEventos() override;
    void cargarDatos() override;
    bool validarEstado() override;
    void registrarEvento(const QString& descripcion) override;

signals:
    void bloqueoFinalizado();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private slots:
    void actualizarCuentaRegresiva();

private:
    int m_lockSeconds;
    int m_remaining;

    QLabel* m_titulo;
    QLabel* m_detalle;
    QProgressBar* m_progress;
    QTimer* m_timer;
};
