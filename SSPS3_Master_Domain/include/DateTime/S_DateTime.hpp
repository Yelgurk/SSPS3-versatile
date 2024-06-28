#ifndef DateTime_hpp
#define DateTime_hpp

#include <iostream>
#include <iomanip>
#include <functional>
#include <stdexcept>
#include <Arduino.h>
#include "./S_Date.hpp"
#include "./S_Time.hpp"

using namespace std;

class S_DateTime
{
private:
    S_Date date;
    S_Time time;

public:
    S_DateTime(const S_Date& d, const S_Time& t) : date(d), time(t) {}
    S_DateTime(int day, int month, int year, int hours, int minutes, int seconds)
        : date(day, month, year), time(hours, minutes, seconds,
                                      [this]() { this->date.setDay(this->date.getDay() - 1); },
                                      [this]() { this->date.setDay(this->date.getDay() + 1); }) {}

    S_Date getDate() const { return date; }
    S_Time getTime() const { return time; }

    void setDate(const S_Date& d) { date = d; }
    void setTime(const S_Time& t) { time = t; }

    string toString(bool new_line = false) { return date.toString() + (new_line ? "\n" : " ") + time.toString(); }

    // Operator overloading with S_DateTime
    S_DateTime operator+(const S_DateTime& other) const {
        return S_DateTime(date + other.date, time + other.time);
    }

    S_DateTime operator-(const S_DateTime& other) const {
        return S_DateTime(date - other.date, time - other.time);
    }

    S_DateTime& operator+=(const S_DateTime& other) {
        date += other.date;
        time += other.time;
        return *this;
    }

    S_DateTime& operator-=(const S_DateTime& other) {
        date -= other.date;
        time -= other.time;
        return *this;
    }

    // Operator overloading with S_Time
    S_DateTime operator+(const S_Time& t) const {
        return S_DateTime(date, time + t);
    }

    S_DateTime operator-(const S_Time& t) const {
        return S_DateTime(date, time - t);
    }

    S_DateTime& operator+=(const S_Time& t) {
        time += t;
        return *this;
    }

    S_DateTime& operator-=(const S_Time& t) {
        time -= t;
        return *this;
    }

    // Operator overloading with S_Date
    S_DateTime operator+(const S_Date& d) const {
        return S_DateTime(date + d, time);
    }

    S_DateTime operator-(const S_Date& d) const {
        return S_DateTime(date - d, time);
    }

    S_DateTime& operator+=(const S_Date& d) {
        date += d;
        return *this;
    }

    S_DateTime& operator-=(const S_Date& d) {
        date -= d;
        return *this;
    }

    bool operator==(const S_DateTime& other) const {
        return date == other.date && time == other.time;
    }

    bool operator!=(const S_DateTime& other) const {
        return !(*this == other);
    }

    bool operator<(const S_DateTime& other) const {
        if (date < other.date) return true;
        if (date > other.date) return false;
        return time < other.time;
    }

    bool operator<=(const S_DateTime& other) const {
        return *this < other || *this == other;
    }

    bool operator>(const S_DateTime& other) const {
        return !(*this <= other);
    }

    bool operator>=(const S_DateTime& other) const {
        return !(*this < other);
    }

    long differenceInSeconds(const S_DateTime& other) const {
        return date.differenceInDays(other.date) * 24 * 3600 + time.differenceInSeconds(other.time);
    }

    long differenceInMinutes(const S_DateTime& other) const {
        return differenceInSeconds(other) / 60;
    }

    long differenceInHours(const S_DateTime& other) const {
        return differenceInMinutes(other) / 60;
    }

    long differenceInDays(const S_DateTime& other) const {
        return differenceInHours(other) / 24;
    }
};

#endif