// raylib 3D front-end for B01 Juego 1: Space Runner 3D (60 Hz fixed step).
// All deterministic state lives in sim::World; this file only reads input and renders in 3D.
#include "raylib.h"
#include "sim/sim.h"

namespace {
class Window {
public:
    Window(int w, int h, const char* title) {
        InitWindow(w, h, title);
        SetTargetFPS(120);
    }
    ~Window() { CloseWindow(); }
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    bool should_close() const { return WindowShouldClose(); }
};

Vector3 lerp_v3(const sim::Vec3& a, const sim::Vec3& b, float alpha) {
    return {
        a.x + (b.x - a.x) * alpha,
        a.y + (b.y - a.y) * alpha,
        a.z + (b.z - a.z) * alpha
    };
}
} // namespace

int main() {
    sim::World world = sim::make_world(1, 35);
    const Window window(1280, 720, "B01 - Space Runner 3D (Fixed Step 60 Hz)");
    sim::World previous = world;
    double accumulator = 0.0;
    double last = GetTime();

    while (!window.should_close()) {
        const double now = GetTime();
        accumulator += now - last;
        last = now;

        const sim::Input input{
            IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A),
            IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D),
            IsKeyDown(KEY_UP) || IsKeyDown(KEY_W),
            IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S),
            IsKeyPressed(KEY_SPACE)
        };

        while (accumulator >= sim::kDt) {
            previous = world;
            sim::update(world, input);
            accumulator -= sim::kDt;
        }
        const auto alpha = static_cast<float>(accumulator / sim::kDt);
        const Vector3 ship_pos = lerp_v3(previous.ship.pos(), world.ship.pos(), alpha);

        Camera3D cam{};
        cam.position = {ship_pos.x * 0.6f, ship_pos.y * 0.6f + 6.0f, ship_pos.z + 18.0f};
        cam.target = {ship_pos.x, ship_pos.y, ship_pos.z - 20.0f};
        cam.up = {0.0f, 1.0f, 0.0f};
        cam.fovy = 60.0f;
        cam.projection = CAMERA_PERSPECTIVE;

        BeginDrawing();
        ClearBackground({10, 14, 26, 255});

        BeginMode3D(cam);
        // Draw player spacecraft
        DrawCubeV(ship_pos, {2.2f, 0.8f, 3.0f}, SKYBLUE);
        DrawCubeWiresV(ship_pos, {2.4f, 0.9f, 3.2f}, RAYWHITE);

        // Draw 3D ring gates
        for (const sim::RingGate& rg : world.rings) {
            const Color c = rg.passed ? GREEN : GOLD;
            DrawCircle3D({rg.pos.x, rg.pos.y, rg.pos.z}, rg.r, {1.0f, 0.0f, 0.0f}, 0.0f, c);
        }

        // Draw 3D asteroids
        for (std::size_t i = 0; i < world.asteroids.size(); ++i) {
            const sim::Asteroid& b = world.asteroids[i];
            if (!b.active) continue;
            const sim::Asteroid& a = previous.asteroids[i];
            const Vector3 pos = lerp_v3(a.pos, b.pos, alpha);
            DrawSphereWires(pos, b.r, 8, 8, MAROON);
        }

        // Draw laser bolts
        for (const sim::LaserBolt& l : world.lasers) {
            DrawCubeV({l.pos.x, l.pos.y, l.pos.z}, {0.3f, 0.3f, 2.5f}, LIME);
        }
        EndMode3D();

        DrawText(TextFormat("B01 SPACE RUNNER 3D | tick %llu | score %u | shield %.0f%%",
                            static_cast<unsigned long long>(world.tick), world.score, world.ship.shield()),
                 16, 14, 20, RAYWHITE);
        EndDrawing();
    }
}
