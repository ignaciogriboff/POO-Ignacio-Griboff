# Ejercicio - Lienzo colaborativo en tiempo real

Desarrollar una aplicación de escritorio en Qt que permita dibujar a mano alzada sobre un lienzo, con comportamiento suave y continuo, incorporando sincronización colaborativa mediante el servidor VPS del equipo. La aplicación debe cumplir con los siguientes requisitos:

Requisitos generales
--------------------

- La aplicación debe implementar un lienzo de dibujo utilizando paintEvent.
- El sistema debe permitir dibujo libre a mano alzada, con suavizado visual y control dinámico del trazo.
- El dibujo será colaborativo entre los integrantes del equipo, sincronizado a través de un backend en VPS.

Interacción de dibujo
---------------------

- Click izquierdo (presionado): dibuja sobre el lienzo
- Click derecho (presionado): actúa como goma (borra el dibujo)

El trazo debe ser:
------------------

- Continuo (sin segmentos visibles)
- Suavizado (no deben notarse “líneas rectas” entre puntos)
- Independiente de la velocidad del mouse

Se debe implementar un mecanismo propio de:
-------------------------------------------

- Interpolación de puntos
- Suavizado del trazo
- Color del trazo: se selecciona con teclas numéricas 1 al 9

- Debe generarse una interpolación de colores entre:

  - RGB inicial: (192, 19, 76)
  - RGB final: (24, 233, 199)
  - El cambio debe ser progresivo

- Grosor del trazo:

  - Se controla con la rueda del mouse (scroll)
  - Debe afectar tanto al lápiz como a la goma
  
- Interfaz gráfica

  - Debe existir una barra superior (tipo menú o toolbar) sobre el lienzo
  - Incluir un botón Guardar con estilo visual tipo Metro de Windows

- Implementar un endpoint en el VPS que permita:

  - Guardar el estado del dibujo
  - Recuperar el dibujo actual
  - Funcionalidad:

    - Al presionar Guardar, se envía el dibujo al servidor
    - Al iniciar la aplicación:

      - Se debe recuperar el dibujo almacenado
      - Permitir continuar editando
      - Colaboración en tiempo real
      - El dibujo es compartido entre los integrantes del equipo
      - Reglas:

        - Si un usuario guarda → actualiza el estado en el servidor
        - Los demás usuarios deben poder:

          - Ver los cambios realizados por otros
          - Sin perder lo que están dibujando localmente

      - Condición clave:
      
        - No se debe perder información al guardar

- Se debe implementar una estrategia de:

  - Fusión de trazos (merge)
  - Modelo incremental de dibujo

Arquitectura (obligatoria)
--------------------------

Se debe implementar al menos:

- Una clase para el modelo de dibujo (almacenamiento de trazos)
- Una clase para la vista (renderizado con paintEvent)
- Una clase para la lógica de sincronización (API / VPS)
