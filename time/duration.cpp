#include "duration.hpp"
#include <chrono>
#include <thread>
namespace timer{


Duration::Duration(int64_t nanoseconds)
    : nanoseconds_(nanoseconds) {}

Duration::Duration(double seconds)
    : nanoseconds_(static_cast<int64_t>(seconds * 1e9)) {}

Duration::Duration(uint32_t seconds, uint32_t nanoseconds)
    : nanoseconds_(static_cast<int64_t>(seconds) * 1e9 + nanoseconds) {}

Duration::Duration(const Duration& other)
    : nanoseconds_(other.nanoseconds_) {}

Duration& Duration::operator=(const Duration& other) {
    if (this != &other) {
        nanoseconds_ = other.nanoseconds_;
    }
    return *this;
}

double Duration::to_seconds() const {
    return static_cast<double>(nanoseconds_) / 1e9;
}

int64_t Duration::to_nanoseconds() const {
    return nanoseconds_;
}

bool Duration::is_zero() const {
    return nanoseconds_ == 0;
}

void Duration::sleep() const {
    auto sleep_time = std::chrono::nanoseconds(nanoseconds_);
    std::this_thread::sleep_for(sleep_time);
}

Duration Duration::operator+(const Duration &rhs) const {
    return Duration(nanoseconds_ + rhs.nanoseconds_);
}

Duration Duration::operator-(const Duration &rhs) const {
    return Duration(nanoseconds_ - rhs.nanoseconds_);
}

Duration Duration::operator-() const {
    return Duration(-nanoseconds_);
}

Duration &Duration::operator+=(const Duration &rhs) {
    nanoseconds_ += rhs.nanoseconds_;
    return *this;
}

Duration &Duration::operator-=(const Duration &rhs) {
    nanoseconds_ -= rhs.nanoseconds_;
    return *this;
}

Duration Duration::operator*(double scale) const {
    return Duration(static_cast<int64_t>(nanoseconds_ * scale));
}

Duration &Duration::operator*=(double scale) {
    nanoseconds_ = static_cast<int64_t>(nanoseconds_ * scale);
    return *this;
}

bool Duration::operator==(const Duration &rhs) const {
    return nanoseconds_ == rhs.nanoseconds_;
}

bool Duration::operator!=(const Duration &rhs) const {
    return nanoseconds_ != rhs.nanoseconds_;
}

bool Duration::operator>(const Duration &rhs) const {
    return nanoseconds_ > rhs.nanoseconds_;
}

bool Duration::operator<(const Duration &rhs) const {
    return nanoseconds_ < rhs.nanoseconds_;
}

bool Duration::operator>=(const Duration &rhs) const {
    return nanoseconds_ >= rhs.nanoseconds_;
}

bool Duration::operator<=(const Duration &rhs) const {
    return nanoseconds_ <= rhs.nanoseconds_;
}



}  // namespace timer
