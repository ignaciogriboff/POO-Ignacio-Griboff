#pragma once

#include "appconfig.h"

#include <QObject>

class Pantalla;

class ApplicationController : public QObject {
    Q_OBJECT

public:
    explicit ApplicationController(const AppConfig& config, QObject* parent = nullptr);
    ~ApplicationController() override;

    void iniciar();

private slots:
    void mostrarLogin();
    void mostrarBloqueado();
    void mostrarEditor();

private:
    void cambiarPantalla(Pantalla* nuevaPantalla);

    AppConfig m_config;
    Pantalla* m_pantallaActual;
};
