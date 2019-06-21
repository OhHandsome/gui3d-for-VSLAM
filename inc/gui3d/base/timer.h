#pragma once

#include <chrono>
#include <cstdint>

namespace gui3d {

class Timer {
public:
  using clock = std::chrono::high_resolution_clock;

  Timer() {
    _last_tick = _first_tick = clock::now();
  }

  void tick() {
    _last_tick = clock::now();
  }

  uint64_t timeSinceStart() const {
    using namespace std::chrono;
    return duration_cast<microseconds>(clock::now() - _first_tick).count();
  }

  uint64_t timeSinceTick() const {
    using namespace std::chrono;
    return duration_cast<microseconds>(clock::now() - _last_tick).count();
  }

private:
  clock::time_point _first_tick;
  clock::time_point _last_tick;
};

} // namespace gui