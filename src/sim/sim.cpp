#include "sim/sim.h"
#include <algorithm>

namespace sim {

namespace {
// Small deterministic PRNG (xorshift32): same seed -> same sequence on every platform.
std::uint32_t next(std::uint32_t& s) {
    s ^= s << 13;
    s ^= s >> 17;
    s ^= s << 5;
    return s;
}
float unit(std::uint32_t& s) { return static_cast<float>(next(s) % 10000u) / 10000.0f; }

void bounce_corridor(Asteroid& a, float hx, float hy) {
    if (a.pos.x - a.r < -hx) { a.pos.x = -hx + a.r; a.vel.x = -a.vel.x; }
    if (a.pos.x + a.r >  hx) { a.pos.x =  hx - a.r; a.vel.x = -a.vel.x; }
    if (a.pos.y - a.r < -hy) { a.pos.y = -hy + a.r; a.vel.y = -a.vel.y; }
    if (a.pos.y + a.r >  hy) { a.pos.y =  hy - a.r; a.vel.y = -a.vel.y; }
}
} // namespace

Quat slerp(const Quat& a, const Quat& b_in, float t) {
    const float clamped_t = std::clamp(t, 0.0f, 1.0f);
    Quat b = b_in;
    float cos_omega = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    if (cos_omega < 0.0f) {
        cos_omega = -cos_omega;
        b = {-b.x, -b.y, -b.z, -b.w};
    }
    float k0 = 1.0f - clamped_t;
    float k1 = clamped_t;
    if (1.0f - cos_omega > 1e-4f) {
        const float omega = std::acos(std::clamp(cos_omega, -1.0f, 1.0f));
        const float inv_sin = 1.0f / std::sin(omega);
        k0 = std::sin((1.0f - clamped_t) * omega) * inv_sin;
        k1 = std::sin(clamped_t * omega) * inv_sin;
    }
    Quat out{
        a.x * k0 + b.x * k1,
        a.y * k0 + b.y * k1,
        a.z * k0 + b.z * k1,
        a.w * k0 + b.w * k1
    };
    const float norm = std::sqrt(out.x * out.x + out.y * out.y + out.z * out.z + out.w * out.w);
    if (norm > 1e-8f) {
        out.x /= norm;
        out.y /= norm;
        out.z /= norm;
        out.w /= norm;
    }
    return out;
}

bool ray_sphere_hit(const Vec3& ray_origin, const Vec3& ray_dir_unit, const Vec3& sphere_center, float sphere_radius) {
    const Vec3 oc = sphere_center - ray_origin;
    const float proj = dot(oc, ray_dir_unit);
    if (proj < 0.0f) return false;
    const float dist_sq = length_sq(oc) - proj * proj;
    return dist_sq <= sphere_radius * sphere_radius;
}

bool is_inside_vision_cone(const Vec3& turret_pos, const Vec3& turret_forward_unit, const Vec3& target_pos,
                           float max_range, float cos_half_angle) {
    const Vec3 to_target = target_pos - turret_pos;
    const float d2 = length_sq(to_target);
    if (d2 > max_range * max_range || d2 <= 1e-12f) return false;
    const Vec3 dir = to_target * (1.0f / std::sqrt(d2));
    return dot(turret_forward_unit, dir) >= cos_half_angle;
}

Vec3 ShipController::clamp_velocity(const Vec3& v) const {
    const float speed2 = length_sq(v);
    if (speed2 <= max_speed_ * max_speed_) return v;
    const float scale = max_speed_ / std::sqrt(speed2);
    return v * scale;
}

void ShipController::apply_damage(float dmg) {
    if (dmg <= 0.0f) return;
    shield_ = std::max(0.0f, shield_ - dmg);
}

void ShipController::recharge_shield(float amount) {
    if (amount <= 0.0f) return;
    shield_ = std::min(100.0f, shield_ + amount);
}

void ShipController::steer(float move_x, float move_y, float bounds_x, float bounds_y, float dt) {
    // Normalize diagonal input so moving diagonally is never 41% faster (B01.3 / B01.4 invariant).
    Vec3 wish{move_x, move_y, 0.0f};
    if (length_sq(wish) > 1.0f) {
        wish = normalized(wish);
    }
    constexpr float kLateralSpeed = 28.0f;
    vel_.x = wish.x * kLateralSpeed;
    vel_.y = wish.y * kLateralSpeed;
    vel_ = clamp_velocity(vel_);

    pos_.x = std::clamp(pos_.x + vel_.x * dt, -bounds_x, bounds_x);
    pos_.y = std::clamp(pos_.y + vel_.y * dt, -bounds_y, bounds_y);
    pos_.z += vel_.z * dt;
}

void update(World& world, const Input& input, double dt) {
    const auto fdt = static_cast<float>(dt);
    const float mx = (input.right ? 1.0f : 0.0f) - (input.left ? 1.0f : 0.0f);
    const float my = (input.up ? 1.0f : 0.0f) - (input.down ? 1.0f : 0.0f);

    world.ship.steer(mx, my, world.corridor_half_width - 2.0f, world.corridor_half_height - 2.0f, fdt);

    if (input.fire) {
        world.lasers.push_back({world.ship.pos(), {0.0f, 0.0f, -1.0f}, 140.0f, 1.2f});
    }

    for (LaserBolt& l : world.lasers) {
        l.pos = l.pos + l.dir * (l.speed * fdt);
        l.ttl -= fdt;
    }

    for (Asteroid& a : world.asteroids) {
        if (!a.active) continue;
        a.pos = a.pos + a.vel * fdt;
        bounce_corridor(a, world.corridor_half_width, world.corridor_half_height);

        // Laser hit check (Ray/Sphere or Sphere/Sphere).
        for (LaserBolt& l : world.lasers) {
            if (l.ttl > 0.0f && length_sq(a.pos - l.pos) <= (a.r + 0.6f) * (a.r + 0.6f)) {
                a.active = false;
                l.ttl = 0.0f;
                world.score += 100u;
                break;
            }
        }

        // Ship collision check.
        if (a.active && length_sq(a.pos - world.ship.pos()) <= (a.r + 1.5f) * (a.r + 1.5f)) {
            a.active = false;
            world.ship.apply_damage(25.0f);
        }
    }

    for (RingGate& rg : world.rings) {
        if (!rg.passed && std::abs(world.ship.pos().z - rg.pos.z) < 1.5f) {
            const float dx = world.ship.pos().x - rg.pos.x;
            const float dy = world.ship.pos().y - rg.pos.y;
            if (dx * dx + dy * dy <= rg.r * rg.r) {
                rg.passed = true;
                world.score += 250u;
                world.ship.recharge_shield(10.0f);
            }
        }
    }

    std::erase_if(world.lasers, [](const LaserBolt& l) { return l.ttl <= 0.0f; });
    ++world.tick;
}

World make_world(std::uint32_t seed, int asteroid_count) {
    World w;
    std::uint32_t s = seed ? seed : 1u;
    w.asteroids.reserve(static_cast<std::size_t>(asteroid_count));
    for (int i = 0; i < asteroid_count; ++i) {
        Asteroid a;
        a.pos.x = (unit(s) * 2.0f - 1.0f) * (w.corridor_half_width - 4.0f);
        a.pos.y = (unit(s) * 2.0f - 1.0f) * (w.corridor_half_height - 4.0f);
        a.pos.z = -30.0f - static_cast<float>(i) * 18.0f;
        a.vel.x = (unit(s) - 0.5f) * 24.0f;
        a.vel.y = (unit(s) - 0.5f) * 24.0f;
        a.vel.z = 10.0f + unit(s) * 15.0f;
        a.r = 2.0f + unit(s) * 1.5f;
        w.asteroids.push_back(a);
    }
    w.rings.reserve(10);
    for (int i = 0; i < 10; ++i) {
        RingGate rg;
        rg.pos = {(unit(s) - 0.5f) * 20.0f, (unit(s) - 0.5f) * 10.0f, -45.0f - static_cast<float>(i) * 60.0f};
        rg.r = 6.0f;
        w.rings.push_back(rg);
    }
    return w;
}

} // namespace sim
