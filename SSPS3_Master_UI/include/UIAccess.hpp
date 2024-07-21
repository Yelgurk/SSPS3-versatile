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
    ColostrumDefroster,
    _END
};

enum class PlaceControlIn : uint8_t
{
    Screen,
    Control
};

enum class StyleActivator : uint8_t
{
    None,
    Rectangle,
    Unscrollable,
    Shadow,
    Focus,
    Select
};

#endif