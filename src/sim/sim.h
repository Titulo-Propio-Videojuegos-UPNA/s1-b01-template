#pragma once
#include <cmath>
#include <cstdint>
#include <vector>

namespace sim {

// 3D Vector for B01 Space Runner 3D (mirrors Unreal Engine FVector concepts in pure C++20).
struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    constexpr Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    constexpr Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    constexpr Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
};

inline constexpr float dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline constexpr Vec3 cross(const Vec3& a, const Vec3& b) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

inline constexpr float length_sq(const Vec3& v) {
    return dot(v, v);
}

inline float length(const Vec3& v) {
    return std::sqrt(length_sq(v));
}

inline Vec3 normalized(const Vec3& v) {
    const float len2 = length_sq(v);
    if (len2 <= 1e-12f) return {0.0f, 0.0f, 0.0f};
    const float inv = 1.0f / std::sqrt(len2);
    return v * inv;
}

// Unit quaternion for smooth 3D orientation and missile homing (B01.3 Slerp vs Gimbal Lock).
struct Quat {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;
};

Quat slerp(const Quat& a, const Quat& b, float t);

// Ray-sphere intersection (B01.3 base for UE5 LineTraceSingleByChannel).
bool ray_sphere_hit(const Vec3& ray_origin, const Vec3& ray_dir_unit, const Vec3& sphere_center, float sphere_radius);

// Turret vision cone check using DotProduct without acos() (B01.3).
bool is_inside_vision_cone(const Vec3& turret_pos, const Vec3& turret_forward_unit, const Vec3& target_pos,
                           float max_range, float cos_half_angle);

// Player ship with class invariants (B01.1: speed, shield, energy bounds).
class ShipController {
public:
    ShipController() = default;
    explicit ShipController(Vec3 start_pos) : pos_(start_pos) {}

    const Vec3& pos() const { return pos_; }
    const Vec3& vel() const { return vel_; }
    float shield() const { return shield_; }
    float max_speed() const { return max_speed_; }

    void set_pos(const Vec3& p) { pos_ = p; }
    void set_vel(const Vec3& v) { vel_ = clamp_velocity(v); }
    void apply_damage(float dmg);
    void recharge_shield(float amount);
    void steer(float move_x, float move_y, float bounds_x, float bounds_y, float dt);

private:
    Vec3 clamp_velocity(const Vec3& v) const;

    Vec3 pos_{0.0f, 0.0f, 0.0f};
    Vec3 vel_{0.0f, 0.0f, -40.0f}; // advances along -Z corridor
    float max_speed_ = 60.0f;
    float shield_ = 100.0f;
};

struct Asteroid {
    Vec3 pos{};
    Vec3 vel{};
    float r = 2.5f;
    bool active = true;
};

struct RingGate {
    Vec3 pos{};
    float r = 6.0f;
    bool passed = false;
};

struct LaserBolt {
    Vec3 pos{};
    Vec3 dir{0.0f, 0.0f, -1.0f};
    float speed = 140.0f;
    float ttl = 1.5f;
};

struct Input {
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;
    bool fire = false;
};

struct World {
    float corridor_half_width = 30.0f;
    float corridor_half_height = 18.0f;
    ShipController ship{};
    std::vector<Asteroid> asteroids;
    std::vector<RingGate> rings;
    std::vector<LaserBolt> lasers;
    std::uint32_t score = 0;
    std::uint64_t tick = 0;
};

// Fixed timestep at 60 Hz as mandated in PLAN_DEFINITIVO_2026-10-25.md (B01.2).
inline constexpr double kDt = 1.0 / 60.0;

// Advances the 3D Space Runner world by one deterministic fixed step.
void update(World& world, const Input& input, double dt = kDt);

// Deterministic 3D Space Runner world from a seed.
World make_world(std::uint32_t seed, int asteroid_count);

} // namespace sim
