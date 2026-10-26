cmake_minimum_required(VERSION 3.24)
project(j1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(J1_BUILD_GAME "Build the raylib front-end (needs a display / X11 headers on Linux)" ON)
option(J1_SANITIZE "Build with AddressSanitizer and UndefinedBehaviorSanitizer (GCC/Clang)" OFF)

include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "Do not reconnect to GitHub on every CMake reconfigure after first download")

# Warnings as errors for our own code only.
function(j1_warnings target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4 /WX /permissive-)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror)
  endif()
endfunction()

if(J1_SANITIZE AND NOT MSVC)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all)
  add_link_options(-fsanitize=address,undefined)
endif()

# --- Simulation core: pure C++, no graphics. Testable and benchmarkable headless.
add_library(j1_sim src/sim/sim.cpp src/sim/replay.cpp)
target_include_directories(j1_sim PUBLIC src)
j1_warnings(j1_sim)
# Determinism: never let the compiler fuse a*b+c into an FMA. Clang contracts by default and, with
# -march=native, changes the bits of the simulation (checked 15-oct-2026: replay hash 927fb831... instead of f27d13e6...).
if(NOT MSVC)
  target_compile_options(j1_sim PRIVATE -ffp-contract=off)
else()
  target_compile_options(j1_sim PRIVATE /fp:precise)
endif()

# --- Tests
enable_testing()
FetchContent_Declare(Catch2 GIT_REPOSITORY https://github.com/catchorg/Catch2.git GIT_TAG v3.7.1 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(Catch2)
add_executable(j1_tests tests/test_sim.cpp tests/test_replay.cpp)
target_link_libraries(j1_tests PRIVATE j1_sim Catch2::Catch2WithMain)
j1_warnings(j1_tests)
include(${catch2_SOURCE_DIR}/extras/Catch.cmake)
catch_discover_tests(j1_tests)

# --- Headless benchmark (ticks per second)
add_executable(j1_bench bench/bench_sim.cpp)
target_link_libraries(j1_bench PRIVATE j1_sim)
j1_warnings(j1_bench)

# --- Game front-end (raylib)
if(J1_BUILD_GAME)
  set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(raylib GIT_REPOSITORY https://github.com/raysan5/raylib.git GIT_TAG 5.5 GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(raylib)
  add_executable(j1 src/game/main.cpp)
  target_link_libraries(j1 PRIVATE j1_sim raylib)
  j1_warnings(j1)
endif()
