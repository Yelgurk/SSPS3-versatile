#pragma once
#ifndef S_Time_hpp
#define S_Time_hpp

#include <Arduino.h>

#define TOTAL_SS_IN_DAY     (60 * 60 * 24)

using namespace std;

class S_Time
{
private:
    char buffer[50];

    int hours;
    int minutes;
    int seconds;

    function<void()> decreaseDay;
    function<void()> increaseDay;

    void normalize(int64_t new_time_value_in_ss)
    {
        hours = minutes = seconds = 0;

        while (abs(new_time_value_in_ss) >= TOTAL_SS_IN_DAY)
            if (new_time_value_in_ss < 0)
            {
                if (decreaseDay) decreaseDay();
                new_time_value_in_ss += TOTAL_SS_IN_DAY;
            }
            else
            {
                if (increaseDay) increaseDay();
                new_time_value_in_ss -= TOTAL_SS_IN_DAY;
            }

        if (new_time_value_in_ss < 0)
        {
            if (decreaseDay) decreaseDay();
            new_time_value_in_ss = TOTAL_SS_IN_DAY + new_time_value_in_ss;
        }

        seconds = new_time_value_in_ss % 60;
        minutes = (new_time_value_in_ss / 60) % 60;
        hours   = new_time_value_in_ss / 3600;
    }

public:
    S_Time(int64_t total_ss, function<void()> decreaseDay = NULL, function<void()> increaseDay = NULL)
    {
        set_time_changed_cb(decreaseDay, increaseDay);
        normalize(total_ss);
    }

    S_Time(int h = 0, int m = 0, int s = 0, function<void()> decreaseDay = NULL, function<void()> increaseDay = NULL)
    : hours(h), minutes(m), seconds(s)
    {
        set_time_changed_cb(decreaseDay, increaseDay);
        normalize(get_total_ss());
    }

    S_Time * set_time_changed_cb(function<void()> decreaseDay = NULL, function<void()> increaseDay = NULL)
    {
        if (decreaseDay != NULL)
            this->decreaseDay = decreaseDay;
        if (increaseDay != NULL)
            this->increaseDay = increaseDay;

        return this;
    }

    int get_hours() const { return hours; }
    int get_minutes() const { return minutes; }
    int get_seconds() const { return seconds; }

    void set_hours(int h) { hours = h; normalize(get_total_ss()); }
    void set_minutes(int m) { minutes = m; normalize(get_total_ss()); }
    void set_seconds(int s) { seconds = s; normalize(get_total_ss()); }

    string to_string()
    {
        sprintf(buffer, "%02d:%02d:%02d", hours, minutes, seconds);
        return string(buffer);
    }

    S_Time operator+(const S_Time& other) const
    {
        return S_Time(hours + other.hours, minutes + other.minutes, seconds + other.seconds);
    }

    S_Time operator-(const S_Time& other) const
    {
        return S_Time(hours - other.hours, minutes - other.minutes, seconds - other.seconds);
    }

    S_Time& operator+=(const S_Time& other)
    {
        normalize(get_total_ss() + other.get_total_ss());
        return *this;
    }

    S_Time& operator-=(const S_Time& other)
    {
        normalize(get_total_ss() - other.get_total_ss());
        return *this;
    }

    bool operator==(const S_Time& other) const        { return get_total_ss() == other.get_total_ss(); }
    bool operator!=(const S_Time& other) const        { return !(*this == other); }
    bool operator<(const S_Time& other) const         { return get_total_ss() < other.get_total_ss(); }
    bool operator<=(const S_Time& other) const        { return *this < other || *this == other; }
    bool operator>(const S_Time& other) const         { return !(*this <= other); }
    bool operator>=(const S_Time& other) const        { return !(*this < other); }

    int64_t get_diff_in_seconds(S_Time& other)  { return get_total_ss() - other.get_total_ss(); }
    int64_t get_diff_in_minutes(S_Time& other)  { return get_diff_in_seconds(other) / 60; }
    int64_t get_diff_in_hours(S_Time& other)    { return get_diff_in_minutes(other) / 60; }

    int64_t get_total_ss() const                { return ((hours * 60) + minutes) * 60 + seconds; }
};

#endif