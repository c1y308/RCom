#ifndef TIME_RATE_HPP
#define TIME_RATE_HPP
#include <cstdint>
#include "duration.hpp"
#include "time.hpp"
namespace timer{


class Rate {
 public:
  explicit Rate(double frequency);
  explicit Rate(uint64_t nanoseconds);
  explicit Rate(const Duration&);
  void sleep();
  void reset();
  Duration cycle_time() const;
  Duration expected_cycleTime() const { return expected_cycle_time_; }

 private:
  Time start_;
  Duration expected_cycle_time_;
  Duration actual_cycle_time_;
};


}  // namespace timer

#endif
