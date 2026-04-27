#pragma once
#include <QString>

enum class TpStatus {
    Pendiente,
    EnProgreso,
    Entregado
};

enum class TpPriority {
    Baja,
    Media,
    Alta
};

struct TpItem {
    int id = 0;
    QString titulo;
    TpStatus estado = TpStatus::Pendiente;
    TpPriority prioridad = TpPriority::Media;
};

// Helpers para UI (texto)
QString statusToString(TpStatus s);
QString priorityToString(TpPriority p);
