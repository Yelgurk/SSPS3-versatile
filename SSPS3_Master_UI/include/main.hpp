#include "DS3231.h"
#include "../../SSPS3_Master_Infrastructure/include/main.hpp"
#include "../../SSPS3_Master_Domain/include/main.hpp"
#include "../resource/lv_resources.hpp"
#include "UIService.hpp"
#include "ProgramControl.hpp"
#include "BlowingControl.hpp"
#include "RtTaskManager.hpp"
#include "Filters/FilterValue.hpp"
#include "Watchdogs/AsynchronousMotorWatchdog.hpp"
#include "Watchdogs/ChillingWatchdog.hpp"
#include "Watchdogs/HeatingWatchdog.hpp"
#include "Watchdogs/V380SupplyWatchdog.hpp"
#include "Watchdogs/WaterJacketDrainWatchdog.hpp"

#include "FRAM_DB.hpp"

/* I2C */
#define SDA                         48
#define SCL                         47
#define INT                         12
#define STM_I2C_ADDR                0x30

/* IO index in arr */
#define DIN_BLOWGUN_SENS    7
#define DIN_380V_SIGNAL     6
#define DIN_ASYNC_M_ERROR   5
#define DIN_STOP_SENS       4
#define DIN_WJACKET_SENS    3

#define ADC_TEMPC_PRODUCT   0
#define ADC_TEMPC_WJACKET   1
#define ADC_VOLTAGE_BATT    2

#define DAC_ASYNC_M_SPEED   0

#define REL_HEATERS         7
#define REL_ASYNC_M         6
#define REL_WJACKET_VALVE   5
#define REL_BLOWGUN_PUMP    4

/* RTC */
bool century = false;
bool h12Flag;
bool pmFlag;

/* Filters */
ExponentialSmoothing exp_filter_tempC_product(0.1);
ExponentialSmoothing exp_filter_tempC_wJacket(0.1);
ExponentialSmoothing exp_filter_24v_batt(0.15);

/* Task manager */
RtTaskManager rt_task_manager;

/* Extern values */
extern FilterValue * filter_tempC_product;
extern FilterValue * filter_tempC_wJacket;
extern FilterValue * filter_24v_batt;

extern AsynchronousMotorWatchdog    * async_motor_wd;
extern ChillingWatchdog             * chilling_wd;
extern HeatingWatchdog              * heating_wd;
extern V380SupplyWatchdog           * v380_supply_wd;
extern WaterJacketDrainWatchdog     * wJacket_drain_wd;

extern TwoWire * itcw;
extern DS3231 * rtc;
extern STM32_slave * STM32;
extern ProgramControl * Program_control;
extern BlowingControl * Blowing_control;
extern UIService * UI_service;
extern S_DateTime * dt_rt;

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