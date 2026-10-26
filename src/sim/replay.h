#pragma once
#include "sim/sim.h"
#include <cstdint>
#include <vector>

namespace sim {

// One recorded input per tick. Replaying the same inputs from the same seed
// must give the same final state, bit for bit.
struct Recording {
    std::uint32_t seed = 1;
    int ball_count = 1;
    std::vector<Input> inputs;
};

// FNV-1a hash over the exact bytes of the simulation state.
std::uint64_t state_hash(const World& world);

// Runs a recording headless and returns the final world.
World replay(const Recording& rec);

} // namespace sim
