# Plantilla B01 (Juego 1): *Space Runner 3D* en C++20 a Paso Fijo (`60 Hz`)

Punto de partida oficial del **Bloque `B01` (*Pieza 1 — Juego 1: Arcade Espacial 3D*, sesiones `S1-01` a `S1-10`)**.

- `src/sim/`: simulación 3D pura en C++20 (`kDt = 1.0 / 60.0`), sin dependencias gráficas y 100 % determinista. Incluye `Vec3`, `Quat` (`slerp`), intersección `ray_sphere_hit`, conos de visión por producto escalar (`is_inside_vision_cone`), `ShipController` con invariantes, oleadas contiguas de `Asteroid`, anillos `RingGate` y proyectiles `LaserBolt`.
- `src/game/`: cliente 3D en `raylib 5.5` (`Camera3D`, `BeginMode3D`, interpolación visual $\alpha$ entre el estado previo y el actual).
- `tests/`: pruebas unitarias con `Catch2 v3` (`test_sim.cpp` y `test_replay.cpp`), verificando invariantes de velocidad diagonal, paso fijo, conos de visión, raycasting 3D y determinismo de repetición (*hash* `FNV-1a` de 64 bits con `-ffp-contract=off`).
- `bench/`: benchmark sin ventana (`j1_bench`) que mide pasos por segundo y factor de tiempo real.
- `scripts/`: scripts de paracaídas Git (`sync_checkpoint.bat` y `sync_checkpoint.sh`) para saltar en 15 segundos a cualquier etiqueta de sesión (`s1-01-start` .. `s1-10-end`).
- `.github/workflows/ci.yml`: pipeline de GitHub Actions con caché de dependencias y matriz `g++` (`ASan`+`UBSan`) + `clang++` (`< 350 min/mes` para 25 alumnos).

## Compilación y Verificación

```text
cmake -B build -S .                      # juego 3D + tests + benchmark
cmake -B build -S . -DJ1_BUILD_GAME=OFF  # solo simulación + tests + benchmark (headless / CI)
cmake --build build
ctest --test-dir build --output-on-failure
./build/j1_bench 100000 64
```

Para activar `AddressSanitizer` + `UndefinedBehaviorSanitizer` (GCC/Clang): añadir `-DJ1_SANITIZE=ON`.

## Reenganche Rápido por Sesión (*Git Parachute*)

Si faltaste a una sesión o quieres comparar tu código con el punto de partida exacto de hoy:

```text
scripts\sync_checkpoint.bat s1-06-start
```

El script guarda automáticamente tus cambios locales en una rama `backup-antes-de-s1-06-start`, descarga los tags del repositorio plantilla (`upstream`) y sitúa tu árbol en la etiqueta indicada.

---

## Cápsulas de Sesión y Etiquetas de Paracaídas (`s1-b01-template`)

- **Proyecto / Toybox:** Juego 1 (Pieza 1): «StarFox / Space Runner 3D» — Arcade espacial 3D sobre plantilla visual C++20 que compila en <1 s (nave, anillos, asteroides, torretas y láseres listos para programar su cerebro en C++).
- **Capas 70 / 20 / 10:** • 70 % Base (completado en las 20 h de aula): Arcade 3D compilando sin warnings, movimiento/disparo en clase C++20, RAII sin fugas en ASan, paso fijo a 60 Hz y 5 bugs de la clínica reparados. · • 20 % Reto autónomo: Implementar torreta con cono de visión (DotProduct) y misil guiado con Slerp de cuaterniones + 3 tests Catch2 propios. · • 10 % Extra Portfolio: Sistema de oleadas configurables, sacudida de cámara (screen shake) por lambdas y grabación GIF/vídeo en README.
- **Plataforma docente (vídeo indexado y entregas):** [https://ikasyn.com/titulo-videojuegos](https://ikasyn.com/titulo-videojuegos)

| Sesión | Subbloque | Horas | Título | Tag Inicio (`sync_checkpoint`) | Tag Fin |
|:---:|:---:|:---:|---|---|---|
| `S1-01` | `B01.1` | 2 h | Juego 1 (Arcade Espacial 3D) · Día 1: de struct en C a class C++20 con invariantes | `scripts\sync_checkpoint.bat s1-01-start` | `s1-01-end` |
| `S1-02` | `B01.1` | 2 h | Juego 1 · Disparo de láseres y ciclo de vida determinista con RAII (std::unique_ptr) | `scripts\sync_checkpoint.bat s1-02-start` | `s1-02-end` |
| `S1-03` | `B01.1` | 2 h | Juego 1 · Oleadas en memoria contigua: std::vector, reserve() y su espejo TArray | `scripts\sync_checkpoint.bat s1-03-start` | `s1-03-end` |
| `S1-04` | `B01.2` | 2 h | Juego 1 · Composición de componentes frente a herencia rígida (virtual Tick) | `scripts\sync_checkpoint.bat s1-04-start` | `s1-04-end` |
| `S1-05` | `B01.2` | 2 h | Juego 1 · Eventos desacoplados con Lambdas/Callbacks (semilla de Delegates en UE5) | `scripts\sync_checkpoint.bat s1-05-start` | `s1-05-end` |
| `S1-06` | `B01.2` | 2 h | Juego 1 · Física determinista: acumulador de paso fijo (dt = 1/60 s) | `scripts\sync_checkpoint.bat s1-06-start` | `s1-06-end` |
| `S1-07` | `B01.3` | 2 h | Juego 1 · Mates 3D de combate I: Vectores, DistSquared y conos de visión (DotProduct) | `scripts\sync_checkpoint.bat s1-07-start` | `s1-07-end` |
| `S1-08` | `B01.3` | 2 h | Juego 1 · Mates 3D de combate II: CrossProduct, ejes locales y matrices TRS | `scripts\sync_checkpoint.bat s1-08-start` | `s1-08-end` |
| `S1-09` | `B01.3` | 2 h | Juego 1 · Mates 3D de combate III: Gimbal Lock vs Cuaterniones (Slerp) y Raycasts | `scripts\sync_checkpoint.bat s1-09-start` | `s1-09-end` |
| `S1-10` | `B01.4` | 2 h | Cierre Juego 1 (Pieza 1): Clínica de caza de 5 bugs y defensa en laboratorio | `scripts\sync_checkpoint.bat s1-10-end` | `s1-10-end` |
