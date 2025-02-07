#pragma once
#ifndef T_DATETIME_H
#define T_DATETIME_H

#ifdef DEV_SSPS3_RUN_ON_PLC
    #include <stdexcept>
    #include <limits>
    #include <cmath>
    #include "./Interface/i_struct_to_string.h"
    #include "./t_timespan.h"
#else
    #include <stdexcept>
    #include <limits>
    #include <cmath>
    #include "i_struct_to_string.h"
    #include "t_timespan.h"
#endif

#pragma pack(1)
struct DateTime : public IStructToString
{
private:
    short year_;
    char month_;
    char day_;
    char hour_;
    char minute_;
    char second_;

    bool is_leap_year() { return is_leap_year(year_); }
    bool is_leap_year(int Year) const
    {
        return (Year % 4 == 0 && Year % 100 != 0) || (Year % 400 == 0);
    }

    int days_in_month() { return days_in_month(year_, month_); }
    int days_in_month(int Year, int Month) const
    {
        return (
            28 + ((int)(Month + floor((float)(Month / 8))) % 2)
            + 2 % Month
            + floor((1 + (1 - (Year % 4 + 2) % (Year % 4 + 1)) * ((Year % 100 + 2) % (Year % 100 + 1)) + (1 - (Year % 400 + 2) % (Year % 400 + 1))) / Month)
            + floor(1 / Month)
            - floor(((1 - (Year % 4 + 2) % (Year % 4 + 1)) * ((Year % 100 + 2) % (Year % 100 + 1)) + (1 - (Year % 400 + 2) % (Year % 400 + 1))) / Month)
            );
    }

    char day_of_the_week() { return day_of_the_week(year_, month_, day_); }
    char day_of_the_week(int Year, int Month, int Day)
    {
        if (Month < 3u)
        {
            --Year;
            Month += 10u;
        }
        else
            Month -= 2u;

        return (Day + (31u * Month / 12u) + Year + (Year / 4u) - (Year / 100u) + (Year / 400u)) % 7u;
    }

    short days_from_start_of_year() { return days_from_start_of_year(year_, month_, day_); }
    short days_from_start_of_year(int Year, int Month, int Day)
    {
        int days = 0;

        for (int m = 1; m < Month; m++)
            days += days_in_month(m, Year);

        days += Day;
        return days;
    }

    void normalize()
    {
        while (second_ >= 60)
            second_ -= 60; ++minute_;

        while (second_ < 0)
            second_ += 60; --minute_;

        while (minute_ >= 60)
            minute_ -= 60; ++hour_;

        while (minute_ < 0)
            minute_ += 60; --hour_;

        while (hour_ >= 24)
            hour_ -= 24; ++day_;

        while (hour_ < 0)
            hour_ += 24; --day_;

        if (year_ < 2000)
            year_ = 2000;

        if (month_ < 1)
            month_ = 1;

        while (day_ > days_in_month(year_, month_))
        {
            day_ -= days_in_month(year_, month_);

            ++month_;

            if (month_ > 12)
                month_ = 1; ++year_;
        }

        while (day_ < 1)
        {
            --month_;

            if (month_ < 1)
                month_ = 12; --year_;

            day_ += days_in_month(year_, month_);
        }
    }

    void adjust_from_timespan(const TimeSpan& span) { return adjust_from_timespan(span.total_seconds()); }
    void adjust_from_timespan(int total_seconds)
    {
        int total_minutes = total_seconds / SECONDS_IN_MINUTE;
        int total_hours = total_minutes / MINUTES_IN_HOUR;
        int total_days = total_hours / HOURS_IN_DAY;

        second_ += static_cast<char>(total_seconds % SECONDS_IN_MINUTE);
        minute_ += static_cast<char>(total_minutes % MINUTES_IN_HOUR);
        hour_ += static_cast<char>(total_hours % HOURS_IN_DAY);
        day_ += static_cast<char>(total_days);
        
        normalize();
    }

public:
    DateTime(short year = 2000, char month = 9, char day = 12, char hour = 12, char minute = 0, char second = 0)
        : year_(year), month_(month), day_(day), hour_(hour), minute_(minute), second_(second)
    {
        normalize();
    }

    DateTime& set_datetime(const DateTime& other) { return set_datetime(other.year_, other.month_, other.day_, other.hour_, other.minute_, other.second_); }
    DateTime& set_datetime(short year, char month, char day, char hour = 0, char minute = 0, char second = 0)
    {
        year_ = year;
        month_ = month;
        day_ = day;
        hour_ = hour;
        minute_ = minute;
        second_ = second;

        normalize();

        return *this;
    }

    short year() const  { return year_; }
    char month() const  { return month_; }
    char day() const    { return day_; }
    char hour() const   { return hour_; }
    char minute() const { return minute_; }
    char second() const { return second_; }

    void add_timespan(const TimeSpan& span)         { adjust_from_timespan(span.total_seconds()); }
    void subtract_timespan(const TimeSpan& span)    { adjust_from_timespan(-span.total_seconds()); }

    virtual std::string to_string() override
    {
        static char buffer[20];

        snprintf(
            buffer,
            sizeof(buffer),
            "%04d.%02d.%02d %02d:%02d:%02d",
            year_, month_, day_, hour_, minute_, second_
        );

        return std::string(buffer);
    }

    TimeSpan operator-(const DateTime& other) const
    {
        int days_diff = 0;
        for (short y = other.year_; y < year_; ++y)
            days_diff += is_leap_year(y) ? 366 : 365;

        for (short y = year_; y < other.year_; ++y)
            days_diff -= is_leap_year(y) ? 366 : 365;

        for (char m = 1; m < month_; ++m)
            days_diff += days_in_month(year_, m);

        for (char m = 1; m < other.month_; ++m)
            days_diff -= days_in_month(other.year_, m);

        days_diff += day_ - other.day_;

        int total_seconds = days_diff * HOURS_IN_DAY * MINUTES_IN_HOUR * SECONDS_IN_MINUTE;
        total_seconds += (hour_ - other.hour_) * MINUTES_IN_HOUR * SECONDS_IN_MINUTE;
        total_seconds += (minute_ - other.minute_) * SECONDS_IN_MINUTE;
        total_seconds += (second_ - other.second_);

        return TimeSpan(total_seconds);
    }

    DateTime& operator++()                          { adjust_from_timespan(1);          return *this; }
    DateTime& operator--()                          { adjust_from_timespan(-1);         return *this; }
    DateTime operator++(int)                        { DateTime temp = *this; ++(*this); return temp; }
    DateTime operator--(int)                        { DateTime temp = *this; --(*this); return temp; }

    DateTime operator+(const TimeSpan& span) const  { DateTime temp = *this; temp.add_timespan(span);       return temp; }
    DateTime operator-(const TimeSpan& span) const  { DateTime temp = *this; temp.subtract_timespan(span);  return temp; }
    DateTime& operator+=(const TimeSpan& span)      { add_timespan(span);                                   return *this; }
    DateTime& operator-=(const TimeSpan& span)      { subtract_timespan(span);                              return *this; }

    DateTime operator+(int seconds) const           { return *this + TimeSpan(seconds); }
    DateTime operator-(int seconds) const           { return *this - TimeSpan(seconds); }
    DateTime& operator+=(int seconds)               { return *this += TimeSpan(seconds); }
    DateTime& operator-=(int seconds)               { return *this -= TimeSpan(seconds); }

    DateTime& operator=(const DateTime& other)      { return set_datetime(other); }

    bool operator<(const DateTime& other) const
    {
        return
            std::tie(year_, month_, day_, hour_, minute_, second_) <
            std::tie(other.year_, other.month_, other.day_, other.hour_, other.minute_, other.second_);
    }
    bool operator==(const DateTime& other) const
    {
        return
            year_ == other.year_ &&
            month_ == other.month_ &&
            day_ == other.day_ &&
            hour_ == other.hour_ &&
            minute_ == other.minute_ &&
            second_ == other.second_;
    }
    bool operator!=(const DateTime& other) const    { return !(*this == other); }
    bool operator<=(const DateTime& other) const    { return *this < other || *this == other; }
    bool operator>(const DateTime& other) const     { return !(*this <= other); }
    bool operator>=(const DateTime& other) const    { return !(*this < other); }
};
#pragma pack(pop)

#endif // T_DATETIME_H
