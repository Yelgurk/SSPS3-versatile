#ifndef KeyModel_hpp
#define KeyModel_hpp

#include <Arduino.h>
#include <vector>

using namespace std;
typedef function<void()> KeyAction;

#define SSPS3_V_1_00    1

#if SSPS3_V_1_00 == 1
enum class KeyMap
{
    L_STACK_4   = 13,
    L_STACK_3   = 12,
    L_STACK_2   = 15,
    L_STACK_1   = 14,
    R_STACK_4   = 9,
    R_STACK_3   = 8,
    R_STACK_2   = 11,
    R_STACK_1   = 10,
    LEFT_TOP    = 0,
    LEFT_BOT    = 3,
    RIGHT_TOP   = 4,
    RIGHT_BOT   = 6,
    LEFT        = 1,
    TOP         = 5,
    RIGHT       = 7,
    BOTTOM      = 2,
    L_STACK_4_REL   = L_STACK_4 + 16,
    L_STACK_3_REL   = L_STACK_3 + 16,
    L_STACK_2_REL   = L_STACK_2 + 16,
    L_STACK_1_REL   = L_STACK_1 + 16,
    R_STACK_4_REL   = R_STACK_4 + 16,
    R_STACK_3_REL   = R_STACK_3 + 16,
    R_STACK_2_REL   = R_STACK_2 + 16,
    R_STACK_1_REL   = R_STACK_1 + 16,
    LEFT_TOP_REL    = LEFT_TOP + 16,
    LEFT_BOT_REL    = LEFT_BOT + 16,
    RIGHT_TOP_REL   = RIGHT_TOP + 16,
    RIGHT_BOT_REL   = RIGHT_BOT + 16,
    LEFT_REL        = LEFT + 16,
    TOP_REL         = TOP + 16,
    RIGHT_REL       = RIGHT + 16,
    BOTTOM_REL      = BOTTOM + 16,
    _END            = 32
};
#elif
#endif

class KeyModel
{
private:
    KeyMap key;
    vector<KeyAction> key_action;
    
public:
    KeyModel(KeyMap key, KeyAction OnClick);
    bool trigger(uint8_t triggered_key);
    bool trigger(KeyMap triggered_key);
};

#endif