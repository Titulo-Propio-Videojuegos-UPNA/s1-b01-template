// Sesion S1-10 (B01.4 — Cierre del Juego 1: Clinica de caza de 5 bugs en Space Runner 3D).
// Cinco errores reales de programacion de juegos en C++20 sembrados, uno por funcion:
//   1. Vector diagonal sin normalizar (la nave corre un 41 % mas en diagonal).
//   2. Movimiento dependiente de los FPS (falta multiplicar por dt / paso fijo).
//   3. Copia masiva de std::vector por valor en cada Tick + puntero invalidado tras push_back (use-after-free).
//   4. Fuga de memoria al spawnear proyectiles con new crudo sin liberar (memory leak detectado por ASan/LSan).
//   5. Fallo matematico en la interseccion Rayo-Esfera del laser (acepta objetivos detras de la nave).
// Uso: ./bugs <1..5>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

namespace {

struct Vec3 {
    float x = 0.0f, y = 0.0f, z = 0.0f;
};

// Bug 1 (B01.4): Vector diagonal sin normalizar.
// Si el jugador pulsa Derecha (1) y Arriba (1) a la vez, la magnitud de (1,1,0) es sqrt(2) ~= 1.4142,
// superando la velocidad maxima lateral permitida (28.0 m/s).
float bug_unnormalized_diagonal(float input_x, float input_y, float max_lateral_speed) {
    // BUG: no se normaliza el vector (input_x, input_y) cuando su longitud supera 1.0f.
    const float vx = input_x * max_lateral_speed;
    const float vy = input_y * max_lateral_speed;
    return std::hypot(vx, vy);
}

// Bug 2 (B01.4): Dependencia de FPS (sumar velocidad por fotograma sin multiplicar por dt).
// A 30 FPS durante 1 segundo (30 frames) avanza 300 m; a 120 FPS durante 1 segundo (120 frames) avanza 1200 m.
float bug_fps_dependent_step(int fps, float speed_m_per_s) {
    float z = 0.0f;
    // BUG: falta multiplicar por dt = 1.0f / fps en cada iteracion.
    for (int frame = 0; frame < fps; ++frame) {
        z += speed_m_per_s;
    }
    return z;
}

// Bug 3 (B01.4): Copia innecesaria en Tick + invalidacion de puntero al realojar std::vector (heap-use-after-free).
int bug_tick_copy_and_dangling_ptr(std::vector<int> wave_ids) { // BUG A: pasado por valor en cada Tick
    int* leader = &wave_ids[0];
    for (int i = 0; i < 64; ++i) {
        wave_ids.push_back(i); // BUG B: realoja el buffer contiguo y deja 'leader' colgando
    }
    return *leader;
}

// Bug 4 (B01.4): Fuga de memoria en el spawner de proyectiles (new sin unique_ptr ni delete).
int bug_projectile_leak() {
    auto* laser_damage = new int[128];
    laser_damage[0] = 25;
    const int dmg = laser_damage[0];
    return dmg; // BUG: fuga de 512 bytes por disparo (usar std::unique_ptr o std::vector)
}

// Bug 5 (B01.4): Fallo de rayo en interseccion Rayo-Esfera (no comprueba si la proyeccion es negativa).
// Devuelve impacto positivo aunque el asteroide este DETRAS del cañon de la nave.
bool bug_ray_sphere_behind_ship() {
    const Vec3 ray_origin{0.0f, 0.0f, 0.0f};
    const Vec3 ray_dir{0.0f, 0.0f, -1.0f};       // dispara hacia -Z (delante de la nave)
    const Vec3 asteroid_pos{0.0f, 0.0f, +20.0f}; // asteroide en +Z (detras de la nave)
    const float radius = 2.0f;

    const Vec3 oc{asteroid_pos.x - ray_origin.x, asteroid_pos.y - ray_origin.y, asteroid_pos.z - ray_origin.z};
    const float proj = oc.x * ray_dir.x + oc.y * ray_dir.y + oc.z * ray_dir.z;
    // BUG: falta `if (proj < 0.0f) return false;` antes de elevar proj al cuadrado.
    const float oc_len_sq = oc.x * oc.x + oc.y * oc.y + oc.z * oc.z;
    const float dist_sq = oc_len_sq - proj * proj;
    return dist_sq <= radius * radius;
}

} // namespace

int main(int argc, char** argv) {
    const int n = argc > 1 ? std::atoi(argv[1]) : 0;
    switch (n) {
    case 1:
        std::printf("vel_diagonal=%.2f (esperado <= 28.00)\n", bug_unnormalized_diagonal(1.0f, 1.0f, 28.0f));
        break;
    case 2:
        std::printf("avance_30fps=%.1f vs avance_120fps=%.1f\n",
                    bug_fps_dependent_step(30, 10.0f), bug_fps_dependent_step(120, 10.0f));
        break;
    case 3:
        std::printf("leader=%d\n", bug_tick_copy_and_dangling_ptr({100, 200, 300}));
        break;
    case 4:
        std::printf("dmg=%d\n", bug_projectile_leak());
        break;
    case 5:
        std::printf("hit_behind=%d (esperado 0)\n", bug_ray_sphere_behind_ship() ? 1 : 0);
        break;
    default:
        std::puts("uso: bugs <1..5>");
        return 0;
    }
    return 0;
}
