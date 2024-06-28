#ifndef Time_hpp
#define Time_hpp

#include <iostream>
#include <iomanip>
#include <functional>
#include <stdexcept>
#include <Arduino.h>

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

    void normalize() {
        while (seconds >= 60) {
            seconds -= 60;
            minutes++;
        }
        while (minutes >= 60) {
            minutes -= 60;
            hours++;
        }
        if (hours >= 24) {
            hours -= 24;
            if (increaseDay) increaseDay();
        }
        while (seconds < 0) {
            seconds += 60;
            minutes--;
        }
        while (minutes < 0) {
            minutes += 60;
            hours--;
        }
        while (hours < 0) {
            hours += 24;
            if (decreaseDay) decreaseDay();
        }
    }

public:
    S_Time(int h = 0, int m = 0, int s = 0) : hours(h), minutes(m), seconds(s) {
        normalize();
    }

    S_Time(int h, int m, int s, function<void()> decreaseDay, function<void()> increaseDay)
        : hours(h), minutes(m), seconds(s), decreaseDay(decreaseDay), increaseDay(increaseDay) {
        normalize();
    }

    int getHours() const { return hours; }
    int getMinutes() const { return minutes; }
    int getSeconds() const { return seconds; }

    void setHours(int h) { hours = h; normalize(); }
    void setMinutes(int m) { minutes = m; normalize(); }
    void setSeconds(int s) { seconds = s; normalize(); }

    string toString()
    {
        sprintf(buffer, "%02d:%02d:%02d", hours, minutes, seconds);
        return string(buffer);
    }

    // Operator overloading
    S_Time operator+(const S_Time& other) const {
        return S_Time(hours + other.hours, minutes + other.minutes, seconds + other.seconds, decreaseDay, increaseDay);
    }

    S_Time operator-(const S_Time& other) const {
        return S_Time(hours - other.hours, minutes - other.minutes, seconds - other.seconds, decreaseDay, increaseDay);
    }

    S_Time& operator+=(const S_Time& other) {
        hours += other.hours;
        minutes += other.minutes;
        seconds += other.seconds;
        normalize();
        return *this;
    }

    S_Time& operator-=(const S_Time& other) {
        hours -= other.hours;
        minutes -= other.minutes;
        seconds -= other.seconds;
        normalize();
        return *this;
    }

    bool operator==(const S_Time& other) const {
        return hours == other.hours && minutes == other.minutes && seconds == other.seconds;
    }

    bool operator!=(const S_Time& other) const {
        return !(*this == other);
    }

    bool operator<(const S_Time& other) const {
        if (hours < other.hours) return true;
        if (hours > other.hours) return false;
        if (minutes < other.minutes) return true;
        if (minutes > other.minutes) return false;
        return seconds < other.seconds;
    }

    bool operator<=(const S_Time& other) const {
        return *this < other || *this == other;
    }

    bool operator>(const S_Time& other) const {
        return !(*this <= other);
    }

    bool operator>=(const S_Time& other) const {
        return !(*this < other);
    }

    // Difference in seconds
    long differenceInSeconds(const S_Time& other) const {
        return (hours * 3600 + minutes * 60 + seconds) - (other.hours * 3600 + other.minutes * 60 + other.seconds);
    }

    long differenceInMinutes(const S_Time& other) const {
        return differenceInSeconds(other) / 60;
    }

    long differenceInHours(const S_Time& other) const {
        return differenceInMinutes(other) / 60;
    }
};

#endif