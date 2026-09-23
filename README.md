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

1. Install the compiler toolchain and FLTK 1.3 (runtime + development headers). Use `-wil` instead of `-wi` so these build-only tools are downloaded and loaded for the current session **without** being added to `onboot.lst` — you don't need to load a full compiler toolchain on every boot if you're not compiling every day:

   ```sh
   tce-load -wil compiletc.tcz
   tce-load -wil fltk-1.3-dev.tcz
   ```

   `tce-load` will automatically pull in `fltk-1.3.tcz` and any required X11 dependencies.

2. Verify FLTK is available:

   ```sh
   fltk-config --version
   ```

3. Compile (quick build):

   ```sh
   g++ -O2 -o tinykeyxorg tinykeyxorg.cpp $(fltk-config --cxxflags --ldflags)
   ```

4. Run:

   ```sh
   ./tinykeyxorg
   ```

   You don't need to prefix it with `sudo` — the app will ask for the password graphically if it needs root privileges.

### Optimized, minimal-size build

Since this is meant to run on Tiny Core Linux, where every kilobyte in the base filesystem matters, you can build a much smaller binary by targeting the 32-bit baseline CPU, optimizing for size instead of speed, dropping the exception/RTTI runtime support (the code doesn't use either), and linking with a trimmed-down linker script (`elf_i386.xbn`, part of `binutils`):

```sh
g++ -flto -march=i486 -mtune=i686 -Os -pipe -fno-exceptions -fno-rtti \
    -o tinykeyxorg tinykeyxorg.cpp $(fltk-config --cxxflags --ldflags) \
    -Wl,-T/usr/local/lib/ldscripts/elf_i386.xbn
```

If your `find /usr -iname elf_i386.xbn` reports a different path than the one above, adjust it accordingly.

Then strip the binary further with `sstrip` (from the `sstrip.tcz` extension, loaded the same on-demand way since it's only needed while building):

```sh
tce-load -wil sstrip.tcz
strip --strip-all tinykeyxorg
sstrip tinykeyxorg
ls -la tinykeyxorg
```

`strip --strip-all` removes symbol tables and debug information the running program doesn't need. `sstrip` goes further and removes the ELF section header table itself, which the Linux kernel doesn't need to execute the binary (only debuggers/analysis tools would). Community members have reported binaries as small as ~16.85 KB with this combination.

Acknowledgements: to Juanito from the official Tiny Core Linux forum for his help and for sharing these compilation flags.

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

1. Instalá las herramientas de compilación y FLTK 1.3 (runtime + headers de desarrollo). Usá `-wil` en vez de `-wi` para que estas herramientas, que solo hacen falta al compilar, se descarguen y carguen en la sesión actual **sin** agregarse a `onboot.lst` — no hace falta cargar todo un toolchain de compilación en cada arranque si no vas a compilar todos los días:

   ```sh
   tce-load -wil compiletc.tcz
   tce-load -wil fltk-1.3-dev.tcz
   ```

   `tce-load` va a traer automáticamente `fltk-1.3.tcz` y las dependencias de X11 necesarias.

2. Verificá que FLTK esté disponible:

   ```sh
   fltk-config --version
   ```

3. Compilá (build rápida):

   ```sh
   g++ -O2 -o tinykeyxorg tinykeyxorg.cpp $(fltk-config --cxxflags --ldflags)
   ```

4. Ejecutá:

   ```sh
   ./tinykeyxorg
   ```

   No hace falta anteponer `sudo` — la aplicación pide la contraseña gráficamente si necesita privilegios de root.

### Build optimizada y de tamaño mínimo

Como esto está pensado para correr en Tiny Core Linux, donde cada kilobyte del sistema base importa, se puede lograr un binario bastante más chico apuntando a la CPU base de 32 bits, optimizando por tamaño en vez de velocidad, sacando el soporte de excepciones/RTTI (el código no usa ninguno de los dos), y linkeando con un linker script recortado (`elf_i386.xbn`, parte de `binutils`):

```sh
g++ -flto -march=i486 -mtune=i686 -Os -pipe -fno-exceptions -fno-rtti \
    -o tinykeyxorg tinykeyxorg.cpp $(fltk-config --cxxflags --ldflags) \
    -Wl,-T/usr/local/lib/ldscripts/elf_i386.xbn
```

Si tu `find /usr -iname elf_i386.xbn` te devuelve una ruta distinta a la de arriba, ajustala.

Después, achicá el binario todavía más con `sstrip` (viene de la extensión `sstrip.tcz`, cargada de la misma forma "on-demand" ya que solo hace falta al compilar):

```sh
tce-load -wil sstrip.tcz
strip --strip-all tinykeyxorg
sstrip tinykeyxorg
ls -la tinykeyxorg
```

`strip --strip-all` elimina la tabla de símbolos y la información de debug que el programa no necesita para correr. `sstrip` va un paso más allá y elimina la tabla de section headers del ELF, que el kernel de Linux tampoco necesita para ejecutarlo (solo la usarían debuggers o herramientas de análisis). En la comunidad reportaron binarios de hasta ~16.85 KB con esta combinación.

Agradecimientos: a Juanito del foro oficial de Tiny Core Linux por su ayuda y por compartir estas flags de compilación.

### Licencia

Este proyecto está licenciado bajo la **GNU General Public License v3.0 (GPLv3)**.
Consultá el archivo [`LICENSE`](./LICENSE), o el texto oficial en
<https://www.gnu.org/licenses/gpl-3.0.en.html>, para conocer los términos completos.
