#pragma once
#ifndef S_DateTime_hpp
#define S_DateTime_hpp

#include <Arduino.h>
#include "S_Date.hpp"
#include "S_Time.hpp"

using namespace std;

class S_DateTime
{
private:
    S_Date date;
    S_Time time;

public:
    S_DateTime(const S_Date& d, const int64_t& total_ss)
    :   date(d),
        time
        (
            total_ss,
            [this]() { this->date.set_day(this->date.get_day() - 1); },
            [this]() { this->date.set_day(this->date.get_day() + 1); }
        )
    {}

    S_DateTime(const S_Date& d, const S_Time& t) : date(d), time(t)
    {
        this->time.set_time_changed_cb(
            [this]() { this->date.set_day(this->date.get_day() - 1); },
            [this]() { this->date.set_day(this->date.get_day() + 1); }
        );
    }

    S_DateTime(int day = 1, int month = 1, int year = 1970, int hours = 0, int minutes = 0, int seconds = 0)
    :   date(day, month, year),
        time
        (
            hours, minutes, seconds,
            [this]() { this->date.set_day(this->date.get_day() - 1); },
            [this]() { this->date.set_day(this->date.get_day() + 1); }
        )
    {}

    S_Date * get_date() { return &date; }
    S_Time * get_time() { return &time; }

    void set_date(const S_Date& d) { date = d; }
    void set_time(const S_Time& t)
    {
        time = t;
        time.set_time_changed_cb(
            [this]() { this->date.set_day(this->date.get_day() - 1); },
            [this]() { this->date.set_day(this->date.get_day() + 1); }
        );
    }

    string to_string(bool new_line = false) { return date.to_string() + (new_line ? "\n" : " ") + time.to_string(); }

    // Operator overloading with S_DateTime
    S_DateTime operator+(const S_DateTime& other) const {
        return S_DateTime(date + other.date, time.get_total_ss() + other.time.get_total_ss());
    }

    S_DateTime operator-(const S_DateTime& other) const {
        return S_DateTime(date - other.date, time.get_total_ss() - other.time.get_total_ss());
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
        return S_DateTime(date, time.get_total_ss() + t.get_total_ss());
    }

    S_DateTime operator-(const S_Time& t) const {
        return S_DateTime(date, time.get_total_ss() - t.get_total_ss());
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
        return S_DateTime(date + d, time.get_total_ss());
    }

    S_DateTime operator-(const S_Date& d) const {
        return S_DateTime(date - d, time.get_total_ss());
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

    long difference_in_seconds(S_DateTime& other) {
        return date.get_diff_in_days(other.date) * 24 * 3600 + time.get_diff_in_seconds(other.time);
    }

    long difference_in_minutes(S_DateTime& other) {
        return difference_in_seconds(other) / 60;
    }

    long difference_in_hours(S_DateTime& other) {
        return difference_in_minutes(other) / 60;
    }

    long difference_in_days(S_DateTime& other) {
        return difference_in_hours(other) / 24;
    }
};

#endif