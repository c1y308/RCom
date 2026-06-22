#ifndef TIME_DURATION_HPP
#define TIME_DURATION_HPP
#include<cstdint>

namespace timer{

class Duration{

public:

    Duration() = default;
    explicit Duration(int64_t nanoseconds);
    explicit Duration(double seconds);
    Duration(uint32_t seconds, uint32_t nanoseconds);
    /* 拷贝构造和拷贝赋值默认即可（无指针） */
    Duration(const Duration& other);
    Duration& operator=(const Duration& other);
    ~Duration() = default;


    double to_seconds() const;
    int64_t to_nanoseconds() const;
    bool is_zero() const;
    void sleep() const;

    Duration operator+(const Duration &rhs) const;
    Duration operator-(const Duration &rhs) const;
    Duration operator-() const;
    Duration &operator+=(const Duration &rhs);
    Duration &operator-=(const Duration &rhs);

    Duration operator*(double scale) const;
    Duration &operator*=(double scale);

    bool operator==(const Duration &rhs) const;
    bool operator!=(const Duration &rhs) const;
    bool operator>(const Duration &rhs) const;
    bool operator<(const Duration &rhs) const;
    bool operator>=(const Duration &rhs) const;
    bool operator<=(const Duration &rhs) const;


private:
    int64_t nanoseconds_{0};


};





}


#endif  // TIME_DURATION_HPP
