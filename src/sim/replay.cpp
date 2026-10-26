#include "sim/replay.h"
#include <cstring>

namespace sim {

namespace {
void mix(std::uint64_t& h, const void* data, std::size_t n) {
    const auto* p = static_cast<const unsigned char*>(data);
    for (std::size_t i = 0; i < n; ++i) {
        h ^= p[i];
        h *= 1099511628211ull;
    }
}
} // namespace

std::uint64_t state_hash(const World& world) {
    std::uint64_t h = 1469598103934665603ull;
    mix(h, &world.tick, sizeof world.tick);
    mix(h, &world.score, sizeof world.score);
    const float ship_f[7] = {
        world.ship.pos().x, world.ship.pos().y, world.ship.pos().z,
        world.ship.vel().x, world.ship.vel().y, world.ship.vel().z,
        world.ship.shield()
    };
    mix(h, ship_f, sizeof ship_f);
    for (const Asteroid& a : world.asteroids) {
        const float f[8] = {a.pos.x, a.pos.y, a.pos.z, a.vel.x, a.vel.y, a.vel.z, a.r, a.active ? 1.0f : 0.0f};
        mix(h, f, sizeof f);
    }
    for (const RingGate& rg : world.rings) {
        const float rf[4] = {rg.pos.x, rg.pos.y, rg.pos.z, rg.passed ? 1.0f : 0.0f};
        mix(h, rf, sizeof rf);
    }
    for (const LaserBolt& lb : world.lasers) {
        const float lf[8] = {lb.pos.x, lb.pos.y, lb.pos.z, lb.dir.x, lb.dir.y, lb.dir.z, lb.speed, lb.ttl};
        mix(h, lf, sizeof lf);
    }
    return h;
}

World replay(const Recording& rec) {
    World w = make_world(rec.seed, rec.ball_count);
    for (const Input& in : rec.inputs) update(w, in);
    return w;
}

} // namespace sim
