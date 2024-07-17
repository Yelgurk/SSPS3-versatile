#include "DS3231.h"
#include "../../SSPS3_Master_Infrastructure/include/main.hpp"
#include "../../SSPS3_Master_Domain/include/main.hpp"
#include "../resource/lv_resources.hpp"
#include "UIService.hpp"
#include "RtTaskManager.hpp"
#include "KeyModel.hpp"
#include <vector>

/* I2C */
#define SDA                         48
#define SCL                         47
#define INT                         12
#define STM_I2C_ADDR                0x30
#define FRAM_I2C_ADDR               0x50

/* RTC */
bool century = false;
bool h12Flag;
bool pmFlag;

/* Task manager */
RtTaskManager rt_task_manager;

extern uint8_t FRAM_address;
extern TwoWire * itcw;
extern DS3231 * rtc;
extern STM32_slave * STM32;
extern UIService * UI_service;
extern S_DateTime * dt_rt;


/* STM32 output states */
uint8_t rt_out_speed_async_m = 0;
bool rt_out_state_async_m = false;
bool rt_out_state_heaters = false;
bool rt_out_state_wJacket = false;

/* STM32 input values and functions */
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

/* FOR DEMO */
uint16_t multipl = 0;
uint8_t dac_mult = 0;

struct __attribute__((packed)) TaskDataStruct
{
    bool done = 0;
    uint8_t lap = 0;
    double ms_total = 0;
    double ms_left = 0;
    float tempC = 0;

    std::string to_string() {
        return 
        "(" +
        std::to_string(done) + " | " +
        std::to_string(lap) + " | " +
        std::to_string(tempC) + " | " +
        std::to_string(ms_total) + " | " +
        std::to_string(ms_left) +
        ")";
    }
};

TaskDataStruct defaultTaskData;
S_DateTime defaultDT;

auto& mem_TDS_4 = Storage::allocate<TaskDataStruct>(defaultTaskData);
auto& mem_TDS_2 = Storage::allocate<TaskDataStruct>(defaultTaskData);
auto& mem_Timer1 = Storage::allocate<bool>(false);
auto& mem_Timer2 = Storage::allocate<uint8_t>(255);
auto& mem_Timer3 = Storage::allocate<uint16_t>(255);
auto& mem_TDS_1 = Storage::allocate<TaskDataStruct>(defaultTaskData);
auto& mem_String = Storage::allocate<std::string>("err");
auto& mem_DT = Storage::allocate<S_DateTime>(defaultDT);
auto& mem_Timer4 = Storage::allocate<uint32_t>(255);
auto& mem_Timer5 = Storage::allocate<uint32_t>(255);
auto& mem_TDS_3 = Storage::allocate<TaskDataStruct>(defaultTaskData);
auto& mem_TDS_5 = Storage::allocate<TaskDataStruct>(defaultTaskData);
auto& mem_TDS_6 = Storage::allocate<TaskDataStruct>(defaultTaskData);