# TinyKeyXorg

A small graphical tool to configure the keyboard (`XkbModel`, `XkbLayout`, `XkbVariant`, `XkbOptions`) for Xorg on **Tiny Core Linux**, built with **FLTK 1.3**.

Una pequeña herramienta gráfica para configurar el teclado (`XkbModel`, `XkbLayout`, `XkbVariant`, `XkbOptions`) en Xorg para **Tiny Core Linux**, construida con **FLTK 1.3**.

---

## English

### Description

TinyKeyXorg is a lightweight C++/FLTK 1.3 GUI for **Tiny Core Linux** that writes and persists a `10-keyboard.conf` file under `/usr/local/share/X11/xorg.conf.d/`, so a custom keyboard layout survives reboots on a system that is normally stored in RAM.

It was built as a graphical front-end for a shell script that does the same job from the terminal, and reproduces the exact same steps:

- Lets you pick the keyboard **model**, **layout** (with a dropdown of common layouts — Spanish, Latin American Spanish, English US/UK, French, German, Italian, Portuguese — plus a manual entry option for anything else), **variant** and extra **XKB options**.
- Requests administrator privileges through a graphical password prompt (via `sudo -S`) if not already running as root — no terminal required.
- Creates `/usr/local/share/X11/xorg.conf.d/` if it doesn't exist yet.
- Backs up any existing `10-keyboard.conf` (with a timestamp if a previous backup is already present) before overwriting it.
- Writes the new Xorg keyboard configuration and sets its permissions to `644`.
- Registers the file in `/opt/.filetool.lst` so it is included in Tiny Core's persistence, and runs `filetool.sh -b` to back it up immediately.
- Shows the full log of everything that happened directly in the window, and reminds you to restart for the changes to take full effect.

### Requirements

- Tiny Core Linux (tested on the x86 / 32-bit branch).
- **FLTK 1.3** — the exact major/minor version matters; the UI code targets the FLTK 1.3 API.
- `sudo` available on the system (included by default on Tiny Core).
- `filetool.sh` available on the system (included by default on Tiny Core).

### Building

1. Install the compiler toolchain:

   ```sh
   tce-load -wi compiletc.tcz
   ```

2. Install FLTK 1.3 (runtime + development headers):

   ```sh
   tce-load -wi fltk-1.3-dev.tcz
   ```

   `tce-load` will automatically pull in `fltk-1.3.tcz` and any required X11 dependencies.

3. Verify FLTK is available:

   ```sh
   fltk-config --version
   ```

4. Compile:

   ```sh
   g++ -O2 -o tinykeyxorg tinykeyxorg.cpp $(fltk-config --cxxflags --ldflags)
   ```

5. Run:

   ```sh
   ./tinykeyxorg
   ```

   You don't need to prefix it with `sudo` — the app will ask for the password graphically if it needs root privileges.

### License

This project is licensed under the **GNU General Public License v3.0 (GPLv3)**.
See the [`LICENSE`](./LICENSE) file, or the official text at
<https://www.gnu.org/licenses/gpl-3.0.en.html>, for the full terms.

---

## Español

### Descripción

TinyKeyXorg es una interfaz gráfica liviana en C++/FLTK 1.3 para **Tiny Core Linux** que escribe y hace persistente un archivo `10-keyboard.conf` en `/usr/local/share/X11/xorg.conf.d/`, para que la configuración de teclado sobreviva a los reinicios en un sistema que normalmente vive en RAM.

Fue creada como interfaz gráfica de un script de shell que hace lo mismo desde la terminal, y reproduce exactamente los mismos pasos:

- Permite elegir el **modelo** de teclado, el **layout** (con una lista desplegable de los más comunes — español, español latinoamericano, inglés US/UK, francés, alemán, italiano, portugués — más una opción de ingreso manual para cualquier otro), la **variante** y **opciones XKB** adicionales.
- Pide privilegios de administrador mediante un cuadro de diálogo gráfico (usando `sudo -S`) si no se está ejecutando ya como root — no hace falta usar una terminal.
- Crea `/usr/local/share/X11/xorg.conf.d/` si todavía no existe.
- Hace una copia de seguridad de cualquier `10-keyboard.conf` existente (con marca de tiempo si ya había una copia previa) antes de sobrescribirlo.
- Escribe la nueva configuración de teclado de Xorg y ajusta sus permisos a `644`.
- Registra el archivo en `/opt/.filetool.lst` para que quede incluido en la persistencia de Tiny Core, y ejecuta `filetool.sh -b` para respaldarlo de inmediato.
- Muestra el registro completo de todo el proceso directamente en la ventana, y recuerda reiniciar para que los cambios tengan efecto completo.

### Requisitos

- Tiny Core Linux (probado en la rama x86 / 32 bits).
- **FLTK 1.3** — la versión exacta importa; el código de la interfaz usa la API de FLTK 1.3.
- `sudo` disponible en el sistema (viene por defecto en Tiny Core).
- `filetool.sh` disponible en el sistema (viene por defecto en Tiny Core).

### Compilación

1. Instalá las herramientas de compilación:

   ```sh
   tce-load -wi compiletc.tcz
   ```

2. Instalá FLTK 1.3 (runtime + headers de desarrollo):

   ```sh
   tce-load -wi fltk-1.3-dev.tcz
   ```

   `tce-load` va a traer automáticamente `fltk-1.3.tcz` y las dependencias de X11 necesarias.

3. Verificá que FLTK esté disponible:

   ```sh
   fltk-config --version
   ```

4. Compilá:

   ```sh
   g++ -O2 -o tinykeyxorg tinykeyxorg.cpp $(fltk-config --cxxflags --ldflags)
   ```

5. Ejecutá:

   ```sh
   ./tinykeyxorg
   ```

   No hace falta anteponer `sudo` — la aplicación pide la contraseña gráficamente si necesita privilegios de root.

### Licencia

Este proyecto está licenciado bajo la **GNU General Public License v3.0 (GPLv3)**.
Consultá el archivo [`LICENSE`](./LICENSE), o el texto oficial en
<https://www.gnu.org/licenses/gpl-3.0.en.html>, para conocer los términos completos.
