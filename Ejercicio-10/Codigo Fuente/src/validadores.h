#pragma once

#include <QString>

struct ResultadoValidacion {
    bool valido;
    QString diagnostico;
};

class ValidadorSintaxis {
public:
    virtual ~ValidadorSintaxis() = default;
    virtual QString nombreLenguaje() const = 0;
    virtual ResultadoValidacion validarLinea(const QString& linea) const = 0;
};

class ValidadorCpp final : public ValidadorSintaxis {
public:
    QString nombreLenguaje() const override;
    ResultadoValidacion validarLinea(const QString& linea) const override;
};

class ValidadorPython final : public ValidadorSintaxis {
public:
    QString nombreLenguaje() const override;
    ResultadoValidacion validarLinea(const QString& linea) const override;
};

class ValidadorJava final : public ValidadorSintaxis {
public:
    QString nombreLenguaje() const override;
    ResultadoValidacion validarLinea(const QString& linea) const override;
};
