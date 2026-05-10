#pragma once

#include <QWidget>

class Pantalla : public QWidget {
    Q_OBJECT

public:
    explicit Pantalla(QWidget* parent = nullptr);
    ~Pantalla() override = default;

    virtual void inicializarUI() = 0;
    virtual void conectarEventos() = 0;
    virtual void cargarDatos() = 0;
    virtual bool validarEstado() = 0;
    virtual void registrarEvento(const QString& descripcion) = 0;
};
