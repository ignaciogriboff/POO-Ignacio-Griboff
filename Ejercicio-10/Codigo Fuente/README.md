# Ejercicio08 — Editor Multilenguaje

Aplicación de escritorio Qt6/C++ con arquitectura polimórfica de pantallas, validación de sintaxis por lenguaje, exportación a JPG y panel lateral estilo LinkedIn.

---

## Requisitos

| Herramienta | Versión mínima |
|---|---|
| Windows | 10 / 11 (64-bit) |
| Qt | 6.7.3 (msvc2019_64) |
| CMake | 3.16+ |
| MSVC / Visual Studio | 2019 o 2022 |

---

## Compilar

```powershell
# 1. Crear directorio de build
cmake -S "C:\Ejercicio08_EditorMultilenguaje" `
      -B "C:\Ejercicio08_EditorMultilenguaje\build" `
      -DCMAKE_PREFIX_PATH="C:\Qt\6.7.3\msvc2019_64" `
      -DCMAKE_BUILD_TYPE=Release

# 2. Compilar
cmake --build "C:\Ejercicio08_EditorMultilenguaje\build" --config Release

# 3. Desplegar DLLs de Qt (solo la primera vez)
C:\Qt\6.7.3\msvc2019_64\bin\windeployqt.exe `
    "C:\Ejercicio08_EditorMultilenguaje\build\Release\Ejercicio08_EditorMultilenguaje.exe"

# 4. Copiar config.ini al directorio de ejecución
Copy-Item "C:\Ejercicio08_EditorMultilenguaje\config.ini" `
          "C:\Ejercicio08_EditorMultilenguaje\build\Release\"
```

---

## Ejecutar

```powershell
cd C:\Ejercicio08_EditorMultilenguaje\build\Release
.\Ejercicio08_EditorMultilenguaje.exe
```

O bien hacer doble clic en el `.exe` desde el Explorador de archivos.

---

## Credenciales por defecto

| Campo | Valor |
|---|---|
| Usuario | `admin` |
| Contraseña | `1234` |
| Segundos de bloqueo | `20` |

Configurables en `build\Release\config.ini`.

---

## Estructura del proyecto

```
Ejercicio08_EditorMultilenguaje/
├── src/
│   ├── main.cpp                         # Entrada, tema oscuro, AppConfig + Logger
│   ├── pantalla.h / .cpp                # Clase base abstracta (interfaz polimórfica)
│   ├── login_screen.h / .cpp            # Pantalla de login con bloqueo por intentos
│   ├── blocked_screen.h / .cpp          # Pantalla de bloqueo con cuenta regresiva
│   ├── editor_screen.h / .cpp           # Editor principal con validación y exportación
│   ├── application_controller.h / .cpp  # Controlador de flujo (trabaja con Pantalla*)
│   ├── validadores.h / .cpp             # Jerarquía de validadores de sintaxis
│   ├── appconfig.h / .cpp               # Lectura de config.ini (QSettings)
│   └── logger.h / .cpp                  # Logger singleton thread-safe
├── config.ini                           # Configuración de la aplicación
├── CMakeLists.txt
└── README.md
```

---

## Funcionalidades

### Login
- Autenticación con usuario y contraseña configurables vía `config.ini`.
- Bloqueo temporal tras 3 intentos fallidos con cuenta regresiva.
- Botón **CERRAR** para salir sin ingresar.

### Editor
- **Selector de lenguaje:** C++, Python, Java.
- **Validación por línea:** al salir de una línea se verifica la sintaxis y se resalta en rojo oscuro si hay error (texto en rosa claro para mantener contraste).
- **Barra de diagnóstico:** muestra `> OK` en verde o `> ERROR` con descripción en rojo.
- **Exportar a JPG:** `Ctrl+S` o botón "⬇ EXPORTAR JPG" — guarda todo el código como imagen.
- **Panel lateral:** perfil estilo LinkedIn con habilidades y contacto.

### Validadores de sintaxis (heurísticos)

| Lenguaje | Reglas verificadas |
|---|---|
| C++ | Paréntesis balanceados · sentencias terminadas en `;` |
| Java | Paréntesis balanceados · sentencias terminadas en `;` |
| Python | Sin llaves `{}` · bloques de control terminados en `:` |

### Registro de eventos
Todos los eventos (login, cambio de lenguaje, validaciones, exportaciones) se registran en:
```
build\Release\logs\eventos.log
```

---

## Configuración (`config.ini`)

```ini
[auth]
initial_user=admin
initial_password=1234
lock_seconds=20

[editor]
default_language=Python
export_path=exports/codigo_exportado.jpg
```

---

## Arquitectura

El proyecto aplica polimorfismo de forma estricta:

- `Pantalla` (clase base abstracta) define la interfaz: `inicializarUI()`, `conectarEventos()`, `cargarDatos()`, `validarEstado()`, `registrarEvento()`.
- `LoginScreen`, `BlockedScreen` y `EditorScreen` heredan de `Pantalla` e implementan todos los métodos virtuales puros.
- `ApplicationController` gestiona el flujo de pantallas trabajando **únicamente** contra punteros `Pantalla*`.
- `ValidadorSintaxis` (clase base abstracta) + `ValidadorCpp`, `ValidadorPython`, `ValidadorJava` forman una jerarquía independiente de validación.
- Sobrescritura de eventos Qt: `keyPressEvent`, `mousePressEvent`, `resizeEvent`, `closeEvent`, `focusInEvent`, `focusOutEvent` en las tres pantallas.
