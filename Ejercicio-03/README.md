# Ejercicio - Planificador de trabajos prácticos (Individual)

Construir una app Qt para planificar trabajos prácticos, con login, seguimiento de entregas y persistencia local.

Alcance mínimo:

- Login con validación y usuarios locales en archivo (CSV o JSON).
- Recordar sesión de forma local (archivo simple) para no pedir login en el mismo equipo. Persistencia de 5 minutos (simulación de sesión).
- Ventana principal con tablero de trabajos prácticos en grilla (QGridLayout), con cada fila armada con QLabel y botones de acciones, y filtro por estado/prioridad.
- Alta/edición/eliminación de trabajos prácticos.
- Editor de notas asociado al trabajo práctico con guardado manual.
- Historial de acciones visible en la UI y guardado en archivo.

Condiciones:

- No usar QML.
- Usar QWidget (no QMainWindow).
- Organizar el código en clases (no todo en main.cpp).

Criterios de evaluación:

- Funciona ✔
- Código organizado ✔
- Persistencia correcta ✔
- UI usable ✔
- Explicación oral ✔


Entregable (todo en GitHub):

- Código fuente completo en el repo de estudiante.
- Capturas de: login, tablero con filtros, editor con estado guardado, historial.
- Preparar el repositorio para revisión: README con instrucciones de compilación y ejecución.
- Considerar que se subirán el mismo repositorio todos los ejercicios, por lo que se recomienda organizar el código en carpetas por ejercicio (ej: /Ejercicio01, /Ejercicio02, etc).

