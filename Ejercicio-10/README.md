# Ejercicio 08 - Editor multilenguaje

- Login inicial con usuario ``admin:1234`` y bloqueo temporal tras 3 intentos fallidos.
- Clase base abstracta ``Pantalla`` con interfaz común y clases derivadas concretas: ``Login``, ``EditorPrincipal`` y ``ModoBloqueado``.
- Uso obligatorio de polimorfismo: el flujo de la aplicación debe trabajar contra punteros o referencias de la clase base.
- Definición de funciones virtuales puras en la clase base (por ejemplo: ``inicializarUI()``, ``conectarEventos()``, ``cargarDatos()``, ``validarEstado()`` y ``registrarEvento()``) y sobrescritura obligatoria en cada derivada.
- Editor principal con selector de lenguaje: C++, Python y Java.
- Jerarquía polimórfica para validación de sintaxis:

	- ``ValidadorSintaxis`` (abstracta)
	- ``ValidadorCpp``, ``ValidadorPython`` y ``ValidadorJava`` (derivadas)

- Validación de sintaxis por línea: la verificación debe ejecutarse al abandonar la línea que se está editando.
- Resaltado de error en rojo cuando la línea sea inválida, con mensaje de diagnóstico amigable en la UI.
- Captura y redefinición de eventos en las clases derivadas:

	- ``keyPressEvent`` para atajos del editor.
	- ``mousePressEvent`` para interacción de cursor y selección.
	- ``resizeEvent`` para adaptación visual del contenido.
	- ``closeEvent`` para confirmación de salida y guardado.
	- ``focusInEvent`` y ``focusOutEvent`` para control de edición y disparo de validación.

- Redefinición consciente de eventos: cada pantalla debe implementar un comportamiento distinto según su responsabilidad.
- Registro de eventos en archivo de log con fecha y descripción de cada acción relevante.
- Lectura de configuración desde archivo (usuario inicial, tiempo de bloqueo, lenguaje por defecto y ruta de exportación).
- Soporte offline: la aplicación debe funcionar sin internet y sin dependencias de servicios remotos.
- Uso obligatorio de ``signals/slots`` tal como lo vimos en clase.
- Luego de un usuario válido, se abre la ventana principal en full screen.
- Exportación final a un único archivo JPG que contenga todo el código escrito, en forma legible y respetando saltos de línea.
- Para que no quede vacía la ventana principal, agregar un panel lateral con currículum estilo LinkedIn: foto, descripción breve, habilidades y contacto.
