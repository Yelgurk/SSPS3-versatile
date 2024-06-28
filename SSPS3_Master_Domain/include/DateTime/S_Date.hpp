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

    bool isLeapYear(int y) const {
        return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
    }

    int daysInMonth(int m, int y) const {
        static const int days_in_month[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
        if (m == 2 && isLeapYear(y)) return 29;
        return days_in_month[m - 1];
    }

    void normalize() {
        while (day > daysInMonth(month, year)) {
            day -= daysInMonth(month, year);
            month++;
            if (month > 12) {
                month = 1;
                year++;
            }
        }
        while (day < 1) {
            month--;
            if (month < 1) {
                month = 12;
                year--;
            }
            day += daysInMonth(month, year);
        }
    }

public:
    S_Date(int d = 1, int m = 1, int y = 1970) : day(d), month(m), year(y) {
        normalize();
    }

    int getDay() const { return day; }
    int getMonth() const { return month; }
    int getYear() const { return year; }

    void setDay(int d) { day = d; normalize(); }
    void setMonth(int m) { month = m; normalize(); }
    void setYear(int y) { year = y; normalize(); }

    string toString()
    {
        sprintf(buffer, "%02d.%02d.%04d", day, month, year);
        return string(buffer);
    }

    // Operator overloading
    S_Date operator+(const S_Date& other) const {
        return S_Date(day + other.day, month + other.month, year + other.year);
    }

    S_Date operator-(const S_Date& other) const {
        return S_Date(day - other.day, month - other.month, year - other.year);
    }

    S_Date& operator+=(const S_Date& other) {
        day += other.day;
        month += other.month;
        year += other.year;
        normalize();
        return *this;
    }

    S_Date& operator-=(const S_Date& other) {
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

    long differenceInDays(const S_Date& other) const {
        // This can be optimized using more sophisticated algorithms or libraries
        // For simplicity, we'll assume all years have 365 days and ignore leap years
        return (year - other.year) * 365 + (month - other.month) * 30 + (day - other.day);
    }
};

#endif