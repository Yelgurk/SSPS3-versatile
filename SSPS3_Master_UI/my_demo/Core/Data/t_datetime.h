﻿#pragma once
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

#pragma pack(push, 1)
struct XDateTime : public IStructToString
{
private:
    short year_;
    char month_;
    char day_;
    char hour_;
    char minute_;
    char second_;

    bool is_leap_year() const { return is_leap_year(year_); }
    bool is_leap_year(int Year) const
    {
        return (Year % 4 == 0 && Year % 100 != 0) || (Year % 400 == 0);
    }

    int days_in_month() const { return days_in_month(year_, month_); }
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

    char day_of_the_week() const { return day_of_the_week(year_, month_, day_); }
    char day_of_the_week(int Year, int Month, int Day) const
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

    short days_from_start_of_year() const { return days_from_start_of_year(year_, month_, day_); }
    short days_from_start_of_year(int Year, int Month, int Day) const
    {
        int days = 0;

        for (int m = 1; m < Month; m++)
            days += days_in_month(Year, m);

        days += Day;
        return days;
    }

    void normalize()
    {
        while (second_ >= 60)
        {
            second_ -= 60;
            ++minute_;
        }

        while (second_ < 0)
        {
            second_ += 60;
            --minute_;
        }

        while (minute_ >= 60)
        {
            minute_ -= 60;
            ++hour_;
        }

        while (minute_ < 0)
        {
            minute_ += 60;
            --hour_;
        }

        while (hour_ >= 24)
        {
            hour_ -= 24;
            ++day_;
        }

        while (hour_ < 0)
        {
            hour_ += 24;
            --day_;
        }

        if (year_ < 2000)
            year_ = 2000;

        if (month_ < 1)
            month_ = 1;

        while (day_ > days_in_month(year_, month_))
        {
            day_ -= days_in_month(year_, month_);

            ++month_;

            if (month_ > 12)
            {
                month_ = 1;
                ++year_;
            }
        }

        while (day_ < 1)
        {
            --month_;

            if (month_ < 1)
            {
                month_ = 12;
                --year_;
            }

            day_ += days_in_month(year_, month_);
        }
    }

    void adjust_from_timespan(const XTimeSpan& span) { return adjust_from_timespan(span.total_seconds()); }
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

    long long to_epoch_seconds() const
    {
        long long days = 0;
        
        for (int y = 2000; y < year_; ++y)
            days += is_leap_year(y) ? 366 : 365;

        for (int y = year_; y < 2000; ++y)
            days -= is_leap_year(y) ? 366 : 365;

        for (int m = 1; m < month_; ++m)
            days += days_in_month(year_, m);

        days += (day_ - 1);

        long long total_seconds =
            days * SECONDS_IN_DAY +
            hour_ * SECONDS_IN_HOUR +
            minute_ * SECONDS_IN_MINUTE +
            second_;

        return total_seconds;
    }

public:
    XDateTime (short year = 2000, char month = 9, char day = 12, char hour = 12, char minute = 0, char second = 0)
        : year_(year), month_(month), day_(day), hour_(hour), minute_(minute), second_(second)
    {
        normalize();
    }

    XDateTime & set_datetime(const XDateTime & other) { return set_datetime(other.year_, other.month_, other.day_, other.hour_, other.minute_, other.second_); }
    XDateTime & set_datetime(short year, char month, char day, char hour = 0, char minute = 0, char second = 0)
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

    void add_timespan(const XTimeSpan& span)         { adjust_from_timespan(span.total_seconds()); }
    void subtract_timespan(const XTimeSpan& span)    { adjust_from_timespan(-span.total_seconds()); }

    virtual std::string to_string() override
    {
        char buffer[20];

        snprintf(
            buffer,
            sizeof(buffer),
            "%04d.%02d.%02d %02d:%02d:%02d",
            year_, month_, day_, hour_, minute_, second_
        );

        return std::string(buffer);
    }

    XTimeSpan operator-(const XDateTime & other) const
    {
        long long diff = this->to_epoch_seconds() - other.to_epoch_seconds();
        return XTimeSpan(static_cast<int>(diff));
    }

    XDateTime & operator++()                          { adjust_from_timespan(1);          return *this; }
    XDateTime & operator--()                          { adjust_from_timespan(-1);         return *this; }
    XDateTime  operator++(int)                        { XDateTime  temp = *this; ++(*this); return temp; }
    XDateTime  operator--(int)                        { XDateTime  temp = *this; --(*this); return temp; }

    XDateTime  operator+(const XTimeSpan& span) const  { XDateTime  temp = *this; temp.add_timespan(span);       return temp; }
    XDateTime  operator-(const XTimeSpan& span) const  { XDateTime  temp = *this; temp.subtract_timespan(span);  return temp; }
    XDateTime & operator+=(const XTimeSpan& span)      { add_timespan(span);                                   return *this; }
    XDateTime & operator-=(const XTimeSpan& span)      { subtract_timespan(span);                              return *this; }

    XDateTime  operator+(int seconds) const           { return *this + XTimeSpan(seconds); }
    XDateTime  operator-(int seconds) const           { return *this - XTimeSpan(seconds); }
    XDateTime & operator+=(int seconds)               { return *this += XTimeSpan(seconds); }
    XDateTime & operator-=(int seconds)               { return *this -= XTimeSpan(seconds); }

    XDateTime & operator=(const XDateTime & other)      { return set_datetime(other); }

    bool operator<(const XDateTime & other) const
    {
        return
            std::tie(year_, month_, day_, hour_, minute_, second_) <
            std::tie(other.year_, other.month_, other.day_, other.hour_, other.minute_, other.second_);
    }
    bool operator==(const XDateTime & other) const
    {
        return
            year_ == other.year_ &&
            month_ == other.month_ &&
            day_ == other.day_ &&
            hour_ == other.hour_ &&
            minute_ == other.minute_ &&
            second_ == other.second_;
    }
    bool operator!=(const XDateTime & other) const    { return !(*this == other); }
    bool operator<=(const XDateTime & other) const    { return *this < other || *this == other; }
    bool operator>(const XDateTime & other) const     { return !(*this <= other); }
    bool operator>=(const XDateTime & other) const    { return !(*this < other); }
};
#pragma pack(pop)

#endif // T_DATETIME_H
