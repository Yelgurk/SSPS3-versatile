#ifndef UIAccess_hpp
#define UIAccess_hpp

#include <Arduino.h>

enum class EquipmentType : uint8_t
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

enum class PlaceControlIn : uint8_t
{
    Screen,
    Control
};

enum class StyleActivator : uint8_t
{
    Rectangle,
    Unscrollable,
    Shadow,
    Focus,
    Select
};

#endif