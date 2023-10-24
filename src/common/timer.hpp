#pragma once

#include <chrono>

namespace W3D {

/*
* We'll use our own simple timer for managing our application's framerate. It
* is a variable frame timer and the aim is to scale movements according to the
* framerate.
*/
class Timer {
  public:
    // THESE HELP WITH TIMER CONVERSIONS
    using Seconds = std::ratio<1>;
    using Milliseconds = std::ratio<1, 1000>;
    using Microseconds = std::ratio<1, 1000000>;
    using Nanoseconds = std::ratio<1, 1000000000>;

    // WE ARE USING THE C++ STANDARD CLOCK
    using Clock = std::chrono::steady_clock;
	using DefaultResolution    = Seconds;

	bool running_ = false;
	Clock::time_point start_time_;
	Clock::time_point previous_tick_;

    /*
    * Default constructor simply gets the initial time but does not start the timer.
    */
    Timer();

    /*
    * There is nothing to destroy as this object manages no pointers.
    */
    virtual ~Timer() = default;

    /*
    * Accessor method to check to see if the game loop is currently running or if it has been paused.
    */
	bool is_running() const;

    /*
    * Starts this timer, getting the latest time and setting it to running.
    */
    void start();

    /*
    * This inlined implementation stops this timer, returning the amount of time
    * that has passed since the previous tick.
    */
    template <typename T = DefaultResolution>
    double stop() {
        auto now = Clock::now();
        auto duration = std::chrono::duration<double, T>(now - previous_tick_);
        previous_tick_ = now;
        return duration.count();
    }

    /*
    * This inlined function computes and returns the duration of the timer, i.e. how long
    * has it been running since it was started.
    */
    template <typename T = DefaultResolution>
    double elapsed() {
        if (!running_) {
            return 0;
        }

        Clock::time_point start = start_time_;
        return std::chrono::duration<double, T>(Clock::now() - start).count();
    }

    /*
    * This inlined function advances the timer, which we should do each
    * iteration of the game application loop.
    */
    template <typename T = DefaultResolution>
    double tick() {
        auto now = Clock::now();
        auto duration = std::chrono::duration<double, T>(now - previous_tick_);
        previous_tick_ = now;
        return duration.count();
    }

}; // class Timer

}  // namespace W3D