# J1 · Plantilla mínima de bucle de paso fijo (S1, bloque `B02.1` — legado P3: `1.4`)

Hay dos versiones del mismo programa, de unas 20 líneas cada una: una pelota que rebota, con **paso fijo de 1/120 s, acumulador e interpolación al dibujar**. Sirven para el pendiente `P03` de `plan-maestro.xlsx` (legado P3: pendiente 3, biblioteca mínima de `B02`). La decisión está en [`../../../planes/2026-10-15_P3_pendiente-3_biblioteca-y-recuperacion.md`](../../../planes/2026-10-15_P3_pendiente-3_biblioteca-y-recuperacion.md).

| Carpeta | Biblioteca | Versión |
|---|---|---|
| `raylib/` | raylib (API en C) | 5.5 |
| `sfml/` | SFML (API en C++17) | 3.1.0 |

Las dos descargan la biblioteca con `FetchContent` y no hace falta instalar nada aparte.

```text
cmake -B build -S raylib     # o -S sfml
cmake --build build
```

**Probado el 15-oct-2026:**
- **Windows 11, g++ 16.1 (MSYS2 UCRT64), «MinGW Makefiles»:** las dos compilan desde cero (raylib en 69 s, SFML en 100 s) y abren la ventana. Con MinGW, el PATH tiene que incluir `msys64/ucrt64/bin` para que el ejecutable encuentre `libgcc` y `libstdc++`.
- **WSL Ubuntu:** las dos fallan al configurar porque faltan las cabeceras de X11 (`libx11-dev`, `libxrandr-dev`…). Hay que instalarlas en el aula y en los equipos del alumnado (pendiente `P04`, legado P3: pendiente 8).
- **Visual Studio 2022:** sin probar.
