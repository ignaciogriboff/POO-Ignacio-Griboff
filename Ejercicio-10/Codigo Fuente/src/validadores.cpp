#include "validadores.h"

namespace {
ResultadoValidacion ok(const QString& msg = "Linea valida") {
    return {true, msg};
}

ResultadoValidacion fail(const QString& msg) {
    return {false, msg};
}

bool terminaConSemicolonCuandoCorresponde(const QString& s) {
    if (s.isEmpty() || s.startsWith("//") || s.endsWith("{") || s.endsWith("}") || s.startsWith("#")) {
        return true;
    }
    if (s.startsWith("if") || s.startsWith("for") || s.startsWith("while") || s.startsWith("switch") || s.startsWith("else")) {
        return true;
    }
    return s.endsWith(";");
}
}

QString ValidadorCpp::nombreLenguaje() const {
    return "C++";
}

ResultadoValidacion ValidadorCpp::validarLinea(const QString& linea) const {
    const QString t = linea.trimmed();
    if (t.isEmpty()) {
        return ok("Linea vacia");
    }

    if (t.count('(') != t.count(')')) {
        return fail("Parentesis desbalanceados en C++");
    }

    if (!terminaConSemicolonCuandoCorresponde(t)) {
        return fail("En C++ suele faltar ';' al final de la sentencia");
    }

    return ok();
}

QString ValidadorPython::nombreLenguaje() const {
    return "Python";
}

ResultadoValidacion ValidadorPython::validarLinea(const QString& linea) const {
    const QString t = linea.trimmed();
    if (t.isEmpty()) {
        return ok("Linea vacia");
    }

    if (t.contains('{') || t.contains('}')) {
        return fail("Python no utiliza llaves para bloques");
    }

    if ((t.startsWith("if ") || t.startsWith("for ") || t.startsWith("while ") || t.startsWith("def ")) && !t.endsWith(':')) {
        return fail("Las sentencias de bloque en Python deben terminar con ':'");
    }

    return ok();
}

QString ValidadorJava::nombreLenguaje() const {
    return "Java";
}

ResultadoValidacion ValidadorJava::validarLinea(const QString& linea) const {
    const QString t = linea.trimmed();
    if (t.isEmpty()) {
        return ok("Linea vacia");
    }

    if (t.count('(') != t.count(')')) {
        return fail("Parentesis desbalanceados en Java");
    }

    if (!terminaConSemicolonCuandoCorresponde(t)) {
        return fail("En Java suele faltar ';' al final de la sentencia");
    }

    return ok();
}
