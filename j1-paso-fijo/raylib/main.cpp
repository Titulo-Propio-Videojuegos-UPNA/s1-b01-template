// Bucle de paso fijo (1/120 s) con acumulador; dibujo con interpolación. raylib 5.5
#include "raylib.h"
struct Ball { float x, y, vx, vy; };
static void step(Ball& b, float dt) {
    b.x += b.vx * dt; b.y += b.vy * dt;
    if (b.x < 0 || b.x > 800) b.vx = -b.vx;
    if (b.y < 0 || b.y > 450) b.vy = -b.vy;
}
int main() {
    InitWindow(800, 450, "J1 - paso fijo");
    const double dt = 1.0 / 120.0; double acc = 0, prev = GetTime();
    Ball b{400, 225, 300, 200}, last = b;
    while (!WindowShouldClose()) {
        double now = GetTime(); acc += now - prev; prev = now;
        while (acc >= dt) { last = b; step(b, float(dt)); acc -= dt; }
        float a = float(acc / dt);
        BeginDrawing(); ClearBackground(RAYWHITE);
        DrawCircle(int(last.x + (b.x - last.x) * a), int(last.y + (b.y - last.y) * a), 10, MAROON);
        EndDrawing();
    }
    CloseWindow();
}
