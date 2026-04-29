#ifndef LOGGER_H
#define LOGGER_H

#include <QString>

class Logger
{
public:
    // Inicializa ruta y crea directorios. Llamar 1 vez al inicio.
    static void init(const QString &appName = "Ej07");

    // Escribe una línea con fecha/hora.
    static void log(const QString &message);

    // (Opcional) log con “tag”
    static void log(const QString &tag, const QString &message);

    // Devuelve el path al archivo para debug
    static QString logFilePath();

private:
    static QString s_logFilePath;
};

#endif // LOGGER_H