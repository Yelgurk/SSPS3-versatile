#pragma once
#ifndef V380SupplyWatchdog_hpp
#define V380SupplyWatchdog_hpp

#include <Arduino.h>
#include <functional>

using namespace std;

class V380SupplyWatchdog
{
private:
    function<void(bool)> set_pause;

public:
    V380SupplyWatchdog(function<void(bool)> set_pause_func) :
    set_pause(set_pause_func)
    {}

    void do_control(bool has_380v_supply) {
        set_pause(!has_380v_supply);
    }
};

#endif