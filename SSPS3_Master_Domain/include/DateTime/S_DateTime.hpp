#pragma once
#ifndef S_DateTime_hpp
#define S_DateTime_hpp

#include <Arduino.h>
#include <tuple>
#include <functional>
#include "S_Date.hpp"
#include "S_Time.hpp"

using namespace std;
using DTLambdaType = function<tuple<uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t>()>;

struct __attribute__((packed)) S_DateTime
{
private:
    S_Date date;
    S_Time time;

    static DTLambdaType get_rt_ds3231;
    static uint8_t hours, minutes, seconds, days, months, years;

    void _set_rt_lambda(DTLambdaType get_rt_ds3231)
    {
        if (get_rt_ds3231 != NULL && !S_DateTime::get_rt_ds3231)
            S_DateTime::get_rt_ds3231 = get_rt_ds3231;
    }

    void _set_rt()
    {
        if (S_DateTime::get_rt_ds3231)
        {
            tie(hours, minutes, seconds, days, months, years) = S_DateTime::get_rt_ds3231();

            time.set_hours(hours);
            time.set_minutes(minutes);
            time.set_seconds(seconds);
            date.set_day(days);
            date.set_month(months);
            date.set_year(((uint16_t)years) + 2000);
        }
    }

public:
    S_DateTime(int day = 1, int month = 1, int year = 2000, int hours = 0, int minutes = 0, int seconds = 0, DTLambdaType get_rt_ds3231 = NULL)
    :   date(day, month, year),
        time
        (
            hours, minutes, seconds,
            [this]() { this->date.set_day(this->date.get_day() - 1); },
            [this]() { this->date.set_day(this->date.get_day() + 1); }
        )
    {
        _set_rt_lambda(get_rt_ds3231);
    }

    S_DateTime(const S_Date& d, const int64_t& total_ss, DTLambdaType get_rt_ds3231 = NULL)
    :   date(d),
        time
        (
            total_ss,
            [this]() { this->date.set_day(this->date.get_day() - 1); },
            [this]() { this->date.set_day(this->date.get_day() + 1); }
        )
    {
        _set_rt_lambda(get_rt_ds3231);
    }

    S_DateTime(const S_Date& d, const S_Time& t, DTLambdaType get_rt_ds3231 = NULL) : date(d), time(t)
    {
        this->time.set_time_changed_cb(
            [this]() { this->date.set_day(this->date.get_day() - 1); },
            [this]() { this->date.set_day(this->date.get_day() + 1); }
        );
        
        _set_rt_lambda(get_rt_ds3231);
    }

    S_Date * get_date() { return &date; }
    S_Time * get_time() { return &time; }

    S_DateTime * get_rt()
    {
        _set_rt();
        return this;
    }

    S_DateTime * set_date(S_Date d)
    {
        date = d;
        return this;
    }

    S_DateTime * set_time(S_Time t)
    {
        time = t;
        time.set_time_changed_cb(
            [this]() { this->date.set_day(this->date.get_day() - 1); },
            [this]() { this->date.set_day(this->date.get_day() + 1); }
        );
        return this;
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
        return S_DateTime(date + S_Date(0, 0, 0, true), time.get_total_ss() + t.get_total_ss());
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

    int64_t difference_in_seconds(S_DateTime& other) {
        return date.get_diff_in_days(other.date) * 24 * 3600 + time.get_diff_in_seconds(other.time);
    }

    int64_t difference_in_minutes(S_DateTime& other) {
        return difference_in_seconds(other) / 60;
    }

    int64_t difference_in_hours(S_DateTime& other) {
        return difference_in_minutes(other) / 60;
    }

    int64_t difference_in_days(S_DateTime& other) {
        return difference_in_hours(other) / 24;
    }
};

DTLambdaType S_DateTime::get_rt_ds3231 = NULL;
uint8_t S_DateTime::hours = 0;
uint8_t S_DateTime::minutes = 0;
uint8_t S_DateTime::seconds = 0;
uint8_t S_DateTime::days = 0;
uint8_t S_DateTime::months = 0;
uint8_t S_DateTime::years = 0;

#endif