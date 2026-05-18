# Ejercicio - T-Rex Extremo

- Inspirado en el clásico juego del T-Rex de Google Chrome.
- El jugador controla un dinosaurio que debe esquivar cactus y pájaros.
- Cada pájaro tiene su **propio `QTimer` independiente**.
- Crear una clase `TRex` que herede de `QWidget` o `QImage`.
- Crear una clase `Pajaro` que herede de `QWidget` o `QImage`:
  
  - Cada instancia de `Pajaro` tiene su propio `QTimer`.
  - El `QTimer` de cada pájaro mueve su posición horizontal a la izquierda.
  
- Utilizar un `QTimer` principal que controle el movimiento de los cactus y detecte colisiones.
- Cada 5 segundos, un nuevo pájaro aparece con su propio `QTimer`.
- Implementar detección de colisiones entre el dinosaurio y los obstáculos (cactus o pájaros).
- El jugador puede saltar, agacharse, adelantarse o frenarse:

  - **Salto**: Presionar la tecla Espacio.
  - **Agacharse**: Presionar la Flecha Abajo.
  - **Adelantarse**: Presionar la Flecha Derecha.
  - **Frenarse**: Presionar la Flecha Izquierda.

- Si el dinosaurio colisiona con un cactus o con un pájaro, el juego termina.
- Mostrar una imagen de "Game Over" y permitir reiniciar el juego.

- Aumentar la velocidad de los cactus y pájaros a medida que pasa el tiempo.
- Diferentes tipos de pájaros con alturas aleatorias.

- Recordar conectar correctamente los `QTimer` usando ``connect(timer, SIGNAL(timeout()), this, SLOT(...))``.
- Utilizar `QTimer::singleShot()` si se quiere programar eventos únicos, como aparición de obstáculos sorpresa.
- Organizar las clases de forma modular para facilitar el crecimiento del proyecto.
