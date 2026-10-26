#include "sim/sim.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using Catch::Matchers::WithinAbs;

TEST_CASE("ship advances along -Z corridor by vel.z * dt in one 60 Hz step") {
    sim::World w;
    sim::update(w, {}, sim::kDt);
    REQUIRE_THAT(w.ship.pos().z, WithinAbs(-40.0 * sim::kDt, 1e-4));
    REQUIRE(w.tick == 1u);
}

TEST_CASE("diagonal steering is normalized and clamped inside the 3D corridor") {
    sim::World w;
    sim::Input diag{false, true, true, false, false};
    sim::update(w, diag, sim::kDt);
    // Lateral speed must not exceed 28 m/s even when pressing both Right and Up.
    const float lateral_speed = std::hypot(w.ship.vel().x, w.ship.vel().y);
    REQUIRE_THAT(lateral_speed, WithinAbs(28.0, 1e-3));
}

TEST_CASE("asteroid bounces off corridor wall and stays inside 3D bounds") {
    sim::World w;
    w.asteroids.push_back({{w.corridor_half_width - 1.0f, 0.0f, -20.0f}, {120.0f, 0.0f, 0.0f}, 2.0f, true});
    sim::update(w, {}, sim::kDt);
    REQUIRE(w.asteroids[0].vel.x < 0.0f);
    REQUIRE(w.asteroids[0].pos.x + w.asteroids[0].r <= w.corridor_half_width);
}

TEST_CASE("turret vision cone (DotProduct) and Ray-Sphere intersection work accurately") {
    const sim::Vec3 turret_pos{0.0f, 0.0f, 0.0f};
    const sim::Vec3 turret_fwd{0.0f, 0.0f, -1.0f};
    const float cos_45 = 0.70710678f;

    REQUIRE(sim::is_inside_vision_cone(turret_pos, turret_fwd, {2.0f, 0.0f, -10.0f}, 30.0f, cos_45));
    REQUIRE_FALSE(sim::is_inside_vision_cone(turret_pos, turret_fwd, {20.0f, 0.0f, -5.0f}, 30.0f, cos_45));

    REQUIRE(sim::ray_sphere_hit({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.5f, 0.0f, -15.0f}, 1.0f));
    REQUIRE_FALSE(sim::ray_sphere_hit({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {5.0f, 0.0f, -15.0f}, 1.0f));
}

TEST_CASE("quaternion slerp interpolates smoothly halfway") {
    const sim::Quat q0{0.0f, 0.0f, 0.0f, 1.0f};
    // 90 deg rotation around Y: sin(45 deg) = cos(45 deg) = 0.70710678
    const sim::Quat q1{0.0f, 0.70710678f, 0.0f, 0.70710678f};
    const sim::Quat mid = sim::slerp(q0, q1, 0.5f);
    // Halfway is 45 deg around Y: sin(22.5 deg) = 0.3826834, cos(22.5 deg) = 0.9238795
    REQUIRE_THAT(mid.y, WithinAbs(0.3826834, 1e-3));
    REQUIRE_THAT(mid.w, WithinAbs(0.9238795, 1e-3));
}

TEST_CASE("make_world is deterministic for a given seed") {
    const auto a = sim::make_world(42, 50);
    const auto b = sim::make_world(42, 50);
    REQUIRE(a.asteroids.size() == 50u);
    for (std::size_t i = 0; i < a.asteroids.size(); ++i) {
        REQUIRE(a.asteroids[i].pos.x == b.asteroids[i].pos.x);
        REQUIRE(a.asteroids[i].vel.y == b.asteroids[i].vel.y);
    }
}
