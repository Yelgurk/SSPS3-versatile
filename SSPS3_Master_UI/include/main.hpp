#include "DS3231.h"
#include "../../SSPS3_Master_Infrastructure/include/main.hpp"
#include "../../SSPS3_Master_Domain/include/main.hpp"
#include "UIService.hpp"
#include "ProgramControl.hpp"
#include "BlowingControl.hpp"
#include "RtTaskManager.hpp"
#include "../resource/lv_resources.hpp"

#include "FRAM_DB.hpp"

#define SDA                         48
#define SCL                         47
#define INT                         12
#define STM_I2C_ADDR                0x30

bool century = false;
bool h12Flag;
bool pmFlag;

extern TwoWire * itcw;
extern DS3231 * rtc;
extern STM32_slave * STM32;
extern ProgramControl * Program_control;
extern BlowingControl * Blowing_control;
extern UIService * UI_service;
extern S_DateTime * dt_rt;

static RtTaskManager rt_task_manager;

uint8_t     Pressed_key = static_cast<uint8_t>(KeyMap::_END);
uint8_t     OptIn_state[8] = { 0 };
uint16_t    AnIn_state[4] = { 0 };

void IRAM_ATTR interrupt_action() {
    interrupted_by_slave = true;
}

void read_input_signals(bool digital_only = false)
{
    static uint8_t index = 0;

    Pressed_key = STM32->get_kb();

    for (index = 0; index < 8; index++)
        OptIn_state[index] = STM32->get(COMM_GET::DGIN, index);

    if (!digital_only)
        for (index = 0; index < 4; index++)
            AnIn_state[index] = STM32->get(COMM_GET::ANIN, index);
}