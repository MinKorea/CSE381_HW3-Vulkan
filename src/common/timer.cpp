// IN THIS FILE WE'LL BE DECLARING METHODS DECLARED INSIDE THIS HEADER FILE
#include "timer.hpp"

namespace W3D {
Timer::Timer() : start_time_(Clock::now()), previous_tick_(Clock::now()) {
}

void Timer::start() {
    if (!running_) {
        running_ = true;
        start_time_ = Clock::now();
    }
}

bool Timer::is_running() const {
    return running_;
}

}  // namespace W3D