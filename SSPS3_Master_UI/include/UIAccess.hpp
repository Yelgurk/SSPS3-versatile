#ifndef UIAccess_hpp
#define UIAccess_hpp

#include <Arduino.h>

enum class UIAccess : uint8_t
{
    All,
    Pasteurizer,
    DairyTaxi,
    DairyTaxiFlowgun,
    DairyTaxiPasteurizer,
    DairyTaxiPasteurizerFlowgun,
    Cheesemaker,
    ColostrumDefroster
};

#endif