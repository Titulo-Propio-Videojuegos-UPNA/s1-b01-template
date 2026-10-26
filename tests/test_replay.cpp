#include "sim/replay.h"
#include <catch2/catch_test_macros.hpp>

namespace {
sim::Recording sample_recording() {
    sim::Recording rec;
    rec.seed = 7;
    rec.ball_count = 50;
    for (int t = 0; t < 60 * 30; ++t) { // 30 simulated seconds at 60 Hz
        sim::Input in;
        in.left = (t / 20) % 2 == 0;
        in.right = (t / 20) % 2 == 1;
        in.up = (t / 15) % 2 == 0;
        in.down = (t / 15) % 2 == 1;
        in.fire = (t % 30) == 0;
        rec.inputs.push_back(in);
    }
    return rec;
}
} // namespace

TEST_CASE("replaying the same recording gives the same state hash") {
    const auto rec = sample_recording();
    const auto h1 = sim::state_hash(sim::replay(rec));
    const auto h2 = sim::state_hash(sim::replay(rec));
    REQUIRE(h1 == h2);
}

TEST_CASE("a different input changes the state hash") {
    auto rec = sample_recording();
    const auto h1 = sim::state_hash(sim::replay(rec));
    rec.inputs[100].right = !rec.inputs[100].right;
    const auto h2 = sim::state_hash(sim::replay(rec));
    REQUIRE(h1 != h2);
}
