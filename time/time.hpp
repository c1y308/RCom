#ifndef TIME_TIME_HPP
#define TIME_TIME_HPP
#include <cstdint>
#include "duration.hpp"
#include <string>
namespace timer {


class Time {

public:
    static const Time MAX;
    static const Time MIN;
    Time() = default;

    explicit Time(uint64_t nanoseconds);
    explicit Time(int nanoseconds);
    explicit Time(double seconds);
    Time(uint32_t seconds, uint32_t nanoseconds);
    Time(const Time &other);
    Time &operator=(const Time &other);

    static Time now();
    static Time mono_time();
    static void sleep_until(const Time &time);
    double to_seconds() const;
    uint64_t to_microseconds() const;
    uint64_t to_nanoseconds() const;
    std::string to_string() const;
    bool is_zero() const;

    Duration operator-(const Time& rhs) const;

    Time operator+(const Duration& rhs) const;
    Time operator-(const Duration& rhs) const;
    Time& operator+=(const Duration& rhs);
    Time& operator-=(const Duration& rhs);

    bool operator==(const Time& rhs) const;
    bool operator!=(const Time& rhs) const;
    bool operator>(const Time& rhs) const;
    bool operator<(const Time& rhs) const;
    bool operator>=(const Time& rhs) const;
    bool operator<=(const Time& rhs) const;

private:
    uint64_t nanoseconds_;

};


}

#endif  // TIME_TIME_HPP
