# Caza de bugs en el Arcade Espacial 3D (`S1`, sesión `S1-10`, subbloque `B01.4`)

`bugs.cpp` contiene los **cinco errores reales de programación de videojuegos** trabajados en la clínica de cierre del Juego 1 (`S1-10`), uno por función. Se ejecuta con `./bugs_san <n>`, donde `n` va de `1` a `5`.

**Tarea del alumnado:** para cada bug, reproducirlo y diagnosticarlo con la herramienta adecuada (`Catch2`, `AddressSanitizer`, `UndefinedBehaviorSanitizer`, `LeakSanitizer`), repararlo en C++20 y **escribir el test de regresión que impida que vuelva a colarse en CI**.

```text
g++ -std=c++20 -O0 -g -fsanitize=address,undefined -fno-omit-frame-pointer bugs.cpp -o bugs_san
for n in 1 2 3 4 5; do ./bugs_san $n; done
```

## Los 5 bugs sembrados en `bugs.cpp` (verificados el `2026-10-26`)

| Bug | Fallo en el juego (`Space Runner 3D`) | Cómo se detecta | Cómo se repara |
|---|---|---|---|
| **1** | **Vector diagonal sin normalizar:** al pulsar $X=1, Y=1$ a la vez, la nave se mueve un $41{,}4\,\%$ más rápido ($\|\vec{v}\| = \sqrt{2} \cdot v_{\max}$) | Test de invariante de velocidad máxima (`Catch2`: `vel.norm() <= max_speed`) | Normalizar el vector de dirección (`dir.normalized()`) cuando su longitud supera `1.0` antes de escalar por `max_speed` |
| **2** | **Movimiento dependiente de los FPS:** integrar `pos += vel` por fotograma sin multiplicar por $\Delta t$ hace que la nave avance el cuádruple a `30 FPS` que a `120 FPS` | Test de equivalencia temporal (`30 pasos × 1/30 s` vs. `120 pasos × 1/120 s`) | Integrar con acumulador de paso fijo (`kDt = 1.0 / 60.0`) multiplicando siempre por `dt` |
| **3** | **Copia masiva en `Tick` + puntero colgante tras `push_back`:** pasar `std::vector<Asteroid>` por valor en cada fotograma y guardar `&asteroids[0]` antes de un `push_back` que realoja el búfer | **ASan:** `ERROR: AddressSanitizer: heap-use-after-free` al leer `locked_target->radius` | Pasar contenedores por referencia (`const std::vector<Asteroid>&`), pre-reservar con `reserve()` y referenciar entidades por índice o ID estable |
| **4** | **Fuga de memoria en spawner de láseres:** reservar rachas de proyectiles con `new LaserBolt[64]` sin liberación ni RAII | **LeakSanitizer (ASan):** `detected memory leaks` al cerrar el ejecutable | Sustituir `new[]` por `std::vector<LaserBolt>` pre-reservado (*object pool*) o `std::unique_ptr` |
| **5** | **Raycast acierta a asteroides situados detrás de la nave:** la intersección Rayo–Esfera olvida comprobar si la proyección `proj < 0.0` | Test unitario de puntería (`Catch2`: disparo hacia $+Z$ con asteroide en $Z = -25$) | Descartar impactos cuya proyección sobre `ray_dir` sea negativa (`if (proj < 0.0) return false;`) |

**Lección clave de `S1-10`:**
- Los sanitizers (`ASan`/`UBSan`/`LSan`) cazan al instante corrupciones de memoria y fugas (bugs 3 y 4), mientras que los bugs matemáticos y de física (bugs 1, 2 y 5) requieren **invariantes verificados con tests deterministas en `Catch2`**.
- Por eso la plantilla de `B01` combina ambos mundos en `.github/workflows/ci.yml`: compilación estricta (`-Wall -Wextra -Wconversion -Werror`) + batería `Catch2` ejecutada bajo `ASan` y `UBSan`.
