#pragma once

#include "appconfig.h"
#include "pantalla.h"

class QLabel;
class QLineEdit;
class QPushButton;

class LoginScreen final : public Pantalla {
    Q_OBJECT

public:
    explicit LoginScreen(const AppConfig& config, QWidget* parent = nullptr);

    void inicializarUI() override;
    void conectarEventos() override;
    void cargarDatos() override;
    bool validarEstado() override;
    void registrarEvento(const QString& descripcion) override;

signals:
    void loginCorrecto();
    void bloqueoSolicitado();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private slots:
    void intentarLogin();

private:
    AppConfig m_config;
    int m_intentosFallidos;

    QLabel* m_titulo;
    QLineEdit* m_usuario;
    QLineEdit* m_password;
    QPushButton* m_btnIngresar;
    QPushButton* m_btnCerrar;
    QLabel* m_estado;
};
