Ejercicio - Tablero Kanban colaborativo (Qt + API)
-----------------------------------------------------

Desarrollar una aplicacion de escritorio en Qt que permita gestionar tareas
tipo Kanban, usando el backend FastAPI del VPS y persistencia en MySQL.

- Consolidar CRUD, orden y movimiento entre columnas.
- Incorporar actualizacion colaborativa.

### Requisitos

- Backend (FastAPI):

  - CRUD de columnas y tarjetas.
  - Endpoint para mover tarjeta entre columnas.
  - Endpoint para reordenar tarjetas en una columna.
  - Autenticacion basica (reutilizar la del VPS).

- Base de datos (MySQL):

  - Tablas: ``columns``, ``cards``, ``card_order``.

- App Qt:

  - Vista Kanban con columnas y tarjetas.
  - Crear/editar/eliminar tarjetas y columnas.
  - Mover tarjetas (drag-and-drop o botones "mover").
  - Actualizacion en tiempo real (polling cada 3-5s o WebSocket).

- Persistencia:

  - Al reiniciar, el tablero queda igual.
