#pragma once
#ifndef T_TIMESPAN_H
#define T_TIMESPAN_H

#ifdef DEV_SSPS3_RUN_ON_PLC
    #include <functional>
    #include "./Interface/i_struct_to_string.h"
    #include "./t_datetime_defines.h"
#else
    #include <functional>
    #include "i_struct_to_string.h"
    #include "t_datetime_defines.h"
#endif

#pragma pack(push, 1)
struct XTimeSpan : public IStructToString
{
private:
    int seconds_;

    bool check_and_set_new_value(int increment)
    {
        long long temp = static_cast<long long>(seconds_) + increment;

        if (temp > INT32_MAX)
        {
            seconds_ = INT32_MAX;
            return false;
        }
        if (temp < INT32_MIN)
        {
            seconds_ = INT32_MIN;
            return false;
        }

        seconds_ = static_cast<int>(temp);
        return true;
    }

public:
    XTimeSpan(int seconds = 0) : seconds_(seconds) {}

    XTimeSpan(int days, int hours, int minutes, int seconds)
        : seconds_(0)
    {
        check_and_set_new_value(
            seconds +
            minutes * SECONDS_IN_MINUTE +
            hours * SECONDS_IN_HOUR +
            days * SECONDS_IN_DAY
        );
    }

    int total_seconds() const       { return seconds_; }
    int total_minutes() const       { return seconds_ / SECONDS_IN_MINUTE; }
    int total_hours() const         { return seconds_ / SECONDS_IN_HOUR; }
    int total_days() const          { return seconds_ / SECONDS_IN_DAY; }

    int total_seconds_abs() const   { return std::abs(total_seconds()); }
    int total_minutes_abs() const   { return std::abs(total_minutes()); }
    int total_hours_abs() const     { return std::abs(total_hours()); }
    int total_days_abs() const      { return std::abs(total_days()); }

    XTimeSpan& inc_seconds(int seconds)  { check_and_set_new_value(seconds);                     return *this; }
    XTimeSpan& inc_minutes(int minutes)  { check_and_set_new_value(minutes * SECONDS_IN_MINUTE); return *this; }
    XTimeSpan& inc_hours(int hours)      { check_and_set_new_value(hours * SECONDS_IN_HOUR);     return *this; }
    XTimeSpan& inc_days(int days)        { check_and_set_new_value(days * SECONDS_IN_DAY);       return *this; }

    XTimeSpan& dec_seconds(int seconds)  { check_and_set_new_value(-seconds);                    return *this; }
    XTimeSpan& dec_minutes(int minutes)  { check_and_set_new_value(-minutes * SECONDS_IN_MINUTE);return *this; }
    XTimeSpan& dec_hours(int hours)      { check_and_set_new_value(-hours * SECONDS_IN_HOUR);    return *this; }
    XTimeSpan& dec_days(int days)        { check_and_set_new_value(-days * SECONDS_IN_DAY);      return *this; }

    virtual std::string to_string() override
    {
        return std::to_string(total_seconds());
    }

    template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    operator T () { return static_cast<T>(total_seconds()); }

    int operator++ () { inc_seconds(1); return total_seconds(); }
    int operator-- () { dec_seconds(1); return total_seconds(); }
    int operator++ (int) { inc_seconds(1); return total_seconds() - 1; }
    int operator-- (int) { dec_seconds(1); return total_seconds() + 1; }

    XTimeSpan operator+(const XTimeSpan& other) const     { return XTimeSpan(seconds_).inc_seconds(other.seconds_); }
    XTimeSpan operator-(const XTimeSpan& other) const     { return XTimeSpan(seconds_).dec_seconds(other.seconds_); }
    XTimeSpan& operator+=(const XTimeSpan& other)         { return inc_seconds(other.seconds_); }
    XTimeSpan& operator-=(const XTimeSpan& other)         { return dec_seconds(other.seconds_); }
    XTimeSpan& operator=(const XTimeSpan& other)          { this->seconds_ = other.seconds_; return *this; }

    bool operator==(const XTimeSpan& other)              { return seconds_ == other.seconds_; }
    bool operator!=(const XTimeSpan& other)              { return seconds_ != other.seconds_; }
    bool operator<(const XTimeSpan& other)               { return seconds_ < other.seconds_; }
    bool operator>(const XTimeSpan& other)               { return seconds_ > other.seconds_; }
    bool operator<=(const XTimeSpan& other)              { return seconds_ <= other.seconds_; }
    bool operator>=(const XTimeSpan& other)              { return seconds_ >= other.seconds_; }

    template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    XTimeSpan operator+(const T& other)                  { return XTimeSpan(seconds_).inc_seconds(other); }

    template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    XTimeSpan operator-(const T& other)                  { return XTimeSpan(seconds_).dec_seconds(other); }

    template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    XTimeSpan& operator+=(const T& other)                { return inc_seconds(other); }

    template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    XTimeSpan& operator-=(const T& other)                { return dec_seconds(other); }

    template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    XTimeSpan& operator=(const T& other)                 { this->seconds_ = other; return *this; }

    template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    XTimeSpan& operator*=(const T& other)                { seconds_ *= other; return *this; }

    template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    XTimeSpan& operator/=(const T& other)                { seconds_ /= (other == 0 ? 1 : other); return *this; }

    template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    bool operator==(const T& other)                     { return seconds_ == other; }

    template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    bool operator!=(const T& other)                     { return seconds_ != other; }

    template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    bool operator<(const T& other)                      { return seconds_ < other; }

    template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    bool operator>(const T& other)                      { return seconds_ > other; }

    template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    bool operator<=(const T& other)                     { return seconds_ <= other; }

    template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    bool operator>=(const T& other)                     { return seconds_ >= other; }
};
#pragma pack(pop)

#endif // !T_TIMESPAN_H
