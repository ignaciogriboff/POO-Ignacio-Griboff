#include "tpitem.h"

QString statusToString(TpStatus s) {
    switch (s) {
    case TpStatus::Pendiente:  return "Pendiente";
    case TpStatus::EnProgreso: return "En progreso";
    case TpStatus::Entregado:  return "Entregado";
    }
    return "Pendiente";
}

QString priorityToString(TpPriority p) {
    switch (p) {
    case TpPriority::Baja:  return "Baja";
    case TpPriority::Media: return "Media";
    case TpPriority::Alta:  return "Alta";
    }
    return "Media";
}
