#ifndef Date_hpp
#define Date_hpp

#include <iostream>
#include <iomanip>
#include <functional>
#include <stdexcept>
#include <Arduino.h>

using namespace std;

class S_Date
{
private:
    char buffer[50];

    int day;
    int month;
    int year;

    bool is_leap_year(int32_t Year) {
        return (Year % 4 == 0 && Year % 100 != 0) || (Year % 400 == 0);
    }

    int days_in_month(int32_t Month, int32_t Year) const
    {
        return (
            28 + ((int32_t)(Month + floor((float)(Month / 8))) % 2)
            + 2 % Month
            + floor((1 + (1 - (Year % 4 + 2) % (Year % 4 + 1)) * ((Year % 100 + 2) % (Year % 100 + 1)) + (1 - (Year % 400 + 2) % (Year % 400 + 1))) / Month)
            + floor(1 / Month)
            - floor(((1 - (Year % 4 + 2) % (Year % 4 + 1)) * ((Year % 100 + 2) % (Year % 100 + 1)) + (1 - (Year % 400 + 2) % (Year % 400 + 1))) / Month)
        );
    }

    uint8_t day_of_the_week() {
        return day_of_the_week(year, month, day); 
    }

    uint8_t day_of_the_week(int32_t Year, int32_t Month, int32_t Day)
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

    uint16_t days_from_start_of_year()
    {
        int days = 0;

        for (int m = 1; m < month; m++)
            days += days_in_month(m, year);
        
        days += day;
        return days;
    }

    void normalize()
    {
        while (day > days_in_month(month, year))
        {
            day -= days_in_month(month, year);
            month++;
            if (month > 12) {
                month = 1;
                year++;
            }
        }

        while (day < 1)
        {
            month--;
            if (month < 1)
            {
                month = 12;
                year--;
            }
            day += days_in_month(month, year);
        }
    }

public:
    S_Date(int d = 1, int m = 1, int y = 1970) : day(d), month(m), year(y) {
        normalize();
    }

    int get_day() const      { return day; }
    int get_month() const    { return month; }
    int get_year() const     { return year; }

    void set_day(int d)      { day = d; normalize(); }
    void set_month(int m)    { month = m; normalize(); }
    void set_year(int y)     { year = y; normalize(); }

    string to_string()
    {
        sprintf(buffer, "%02d.%02d.%04d", day, month, year);
        return string(buffer);
    }

    S_Date operator+(const S_Date& other) const {
        return S_Date(day + other.day, month + other.month, year + other.year);
    }

    S_Date operator-(const S_Date& other) const {
        return S_Date(day - other.day, month - other.month, year - other.year);
    }

    S_Date& operator+=(const S_Date& other)
    {
        day += other.day;
        month += other.month;
        year += other.year;
        normalize();
        return *this;
    }

    S_Date& operator-=(const S_Date& other)
    {
        day -= other.day;
        month -= other.month;
        year -= other.year;
        normalize();
        return *this;
    }

    bool operator==(const S_Date& other) const {
        return day == other.day && month == other.month && year == other.year;
    }

    bool operator!=(const S_Date& other) const {
        return !(*this == other);
    }

    bool operator<(const S_Date& other) const {
        if (year < other.year) return true;
        if (year > other.year) return false;
        if (month < other.month) return true;
        if (month > other.month) return false;
        return day < other.day;
    }

    bool operator<=(const S_Date& other) const {
        return *this < other || *this == other;
    }

    bool operator>(const S_Date& other) const {
        return !(*this <= other);
    }

    bool operator>=(const S_Date& other) const {
        return !(*this < other);
    }

    /*
    int64_t difference_in_days(const S_Date& other) const {
        return (year - other.year) * 365 + (month - other.month) * 30 + (day - other.day);
    }
    */

   int64_t get_diff_in_days(S_Date& other)
   {
        int64_t days_difference =
            days_from_start_of_year() -
            other.days_from_start_of_year();
        
        for (int32_t y = min(year, other.year); y < max(year, other.year); ++y)
            if (is_leap_year(y))
                days_difference += 366;
            else
                days_difference += 365;

        return days_difference;
    }
};

#endif