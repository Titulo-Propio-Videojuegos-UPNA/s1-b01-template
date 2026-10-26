// Headless benchmark: how many fixed steps per second the 3D Space Runner simulation runs without drawing.
// Usage: j1_bench [ticks] [asteroids]
#include "sim/replay.h"
#include "sim/sim.h"
#include <chrono>
#include <cstdio>
#include <cstdlib>

int main(int argc, char** argv) {
    const long ticks = argc > 1 ? std::atol(argv[1]) : 100000L;
    const int asteroids = argc > 2 ? std::atoi(argv[2]) : 64;
    sim::World w = sim::make_world(1, asteroids);
    const sim::Input autopilot{false, true, true, false, true};
    const auto t0 = std::chrono::steady_clock::now();
    for (long i = 0; i < ticks; ++i) sim::update(w, autopilot);
    const auto t1 = std::chrono::steady_clock::now();
    const double s = std::chrono::duration<double>(t1 - t0).count();
    std::printf("ticks=%ld asteroids=%d seconds=%.3f ticks_per_second=%.0f realtime_factor=%.1fx hash=%016llx\n",
                ticks, asteroids, s, static_cast<double>(ticks) / s, (static_cast<double>(ticks) * sim::kDt) / s,
                static_cast<unsigned long long>(sim::state_hash(w)));
}
