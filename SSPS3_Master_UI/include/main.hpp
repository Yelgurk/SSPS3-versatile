#include "DS3231.h"
#include "../../SSPS3_Master_Infrastructure/include/main.hpp"
#include "../../SSPS3_Master_Domain/include/main.hpp"
#include "../resource/lv_resources.hpp"
#include "UIService.hpp"
#include "UIManager.hpp"
#include "ProgramControl.hpp"
#include "BlowingControl.hpp"
#include "RtTaskManager.hpp"
#include "Filters/FilterValue.hpp"
#include "Watchdogs/AsynchronousMotorWatchdog.hpp"
#include "Watchdogs/ChillingWatchdog.hpp"
#include "Watchdogs/HeatingWatchdog.hpp"
#include "Watchdogs/V380SupplyWatchdog.hpp"
#include "Watchdogs/WaterJacketDrainWatchdog.hpp"
#include "Watchdogs/ProgStartupWatchdog.hpp"
#include "FRAM_DB.hpp"

//#define IS_SSPS3F1_V1
#define IS_SSPS3F1_BLACKOUT_EDITION
//#define DEMO_WITHOUT_EXTERNAL_POWER

/* I2C */
#ifdef IS_SSPS3F1_V1
    #define SDA                         48
    #define SCL                         47
    #define INT                         12
#endif

#ifdef IS_SSPS3F1_BLACKOUT_EDITION
    #define SDA                         40
    #define SCL                         39
    #define INT                         38
    #define INT_KB                      21
#endif

#define STM_I2C_ADDR                    0x30
#define FRAM_I2C_ADDR                   0x50

/* SSPS3F1 BLACKOUT EDITION - Extra part */

#ifdef IS_SSPS3F1_BLACKOUT_EDITION
    #define BUZZER_PIN                  11
#endif

/* RTC */
bool century = false;
bool h12Flag;
bool pmFlag;

/* Filters */
ExponentialSmoothing exp_filter_tempC_product(0.06);
ExponentialSmoothing exp_filter_tempC_wJacket(0.06);
ExponentialSmoothing exp_filter_24v_batt(0.03);

/* Task manager */
RtTaskManager rt_task_manager;

/* Extern UI controls (scratch in ProgramControl for Notify) */
extern UINotificationBar * UI_notification_bar;

/* Extern values */
extern FilterValue * filter_tempC_product;
extern FilterValue * filter_tempC_wJacket;
extern FilterValue * filter_24v_batt;

extern uint8_t FRAM_address;
extern TwoWire * itcw;
extern DS3231 * rtc;
extern STM32_slave * STM32;
extern BlowingControl * Blowing_control;
extern UIService * UI_service;
extern UIManager * UI_manager;
extern S_DateTime * dt_rt;

extern AsynchronousMotorWatchdog    * async_motor_wd;
extern ChillingWatchdog             * chilling_wd;
extern HeatingWatchdog              * heating_wd;
extern V380SupplyWatchdog           * v380_supply_wd;
extern WaterJacketDrainWatchdog     * wJacket_drain_wd;
extern ProgStartupWatchdog          * prog_stasrtup_wd;

/* STM32 output states */
uint8_t rt_out_speed_async_m = 0;
bool rt_out_state_async_m = false;
bool rt_out_state_heaters = false;
bool rt_out_state_wJacket = false;

/* STM32 input values and functions */
uint8_t             Pressed_key = static_cast<uint8_t>(KeyMap::_END);
extern boolean      Pressed_key_accept_for_prog;
extern uint8_t      OptIn_state[8];
extern uint16_t     AnIn_state[4];

void IRAM_ATTR interrupt_action() {
    interrupted_by_slave = true;
}

void IRAM_ATTR interrupt_action_2() {
    interrupted_by_kb = true;
}

bool read_digital_signals(bool timer_call = true)
{
    static uint8_t old = 123;

    if (!timer_call)
    {
        Pressed_key = STM32->get_kb();

        if (old != Pressed_key)
        {
            if (Pressed_key >= 0 && Pressed_key < 16 )
                digitalWrite(BUZZER_PIN, HIGH);

            //Serial.println(Pressed_key);
        }
        old = Pressed_key;
    }

    static uint8_t index = 0;
    for (index = 0; index < 8; index++)
    {
        OptIn_state[index] = STM32->get(COMM_GET::DGIN, index);

        //Serial.print(OptIn_state[index]);

        //if (index == 7)
        //    Serial.println();
    }

    if (!var_stop_btn_type.local())
        OptIn_state[DIN_STOP_SENS] = OptIn_state[DIN_STOP_SENS] > 0 ? 0 : 1;

#ifdef DEMO_WITHOUT_EXTERNAL_POWER
    OptIn_state[DIN_WJACKET_SENS]   = 1;
    OptIn_state[DIN_380V_SIGNAL]    = 1;
    OptIn_state[DIN_STOP_SENS]      = 0;
#endif

    return !timer_call;
}

/* 8pt == 1*C */
static float        pt_to_tempC = 8.f;
static uint16_t     offset_1    = pt_to_tempC * 11.f, //offset - отнимает температуру
                    offset_2    = pt_to_tempC * 4.f;
static uint16_t     offset_prog = offset_1 + pt_to_tempC * 3.f;
static uint16_t     result_1    = 0,
                    result_2    = 0;

void debug_show_tempC_offsets()
{
    Serial.print("off_1 = ");
    Serial.println(offset_1 / pt_to_tempC);
    Serial.print("off_2 = ");
    Serial.println(offset_2 / pt_to_tempC);
}

void read_tempC_sensors(bool is_startup_call = false);
void read_analog_signals(bool is_startup_call = false)
{
    static uint8_t index = 0;

    for (index = 0; index < 4; index++)
    {
        if (is_startup_call || (index != ADC_TEMPC_PRODUCT && index != ADC_TEMPC_WJACKET))
            AnIn_state[index] = STM32->get(COMM_GET::ANIN, index);

        //Serial.print(AnIn_state[index]);
        //Serial.print(",  ");

        //if (index == 3)
        //    Serial.println();
    }

    read_tempC_sensors(is_startup_call);

#ifdef DEMO_WITHOUT_EXTERNAL_POWER
    if (is_startup_call)
        for (uint8_t it = 0; it < 10; it++)
        {
            exp_filter_tempC_product.add_value(1250);
            exp_filter_tempC_wJacket.add_value(1250);
        }
#else
    if (is_startup_call)
    {
        for (uint8_t call = 0; call < 10; call++)
        {
            filter_24v_batt->add_value(STM32->get(COMM_GET::ANIN, ADC_VOLTAGE_BATT));
            
            result_1 = STM32->get(COMM_GET::ANIN, ADC_TEMPC_PRODUCT);
            result_2 = STM32->get(COMM_GET::ANIN, ADC_TEMPC_WJACKET);

            exp_filter_tempC_product.add_value(result_1 >= offset_1 ? result_1 - offset_1 : result_1);
            exp_filter_tempC_wJacket.add_value(result_2 >= result_2 ? result_2 - offset_2 : result_2);

            Serial.println(call+1);

            Serial.print("prod: ");
            Serial.print(result_1 - offset_1);
            Serial.print(" = ");
            Serial.println(filter_tempC_product->get_physical_value());

            Serial.print("wjac: ");
            Serial.print(result_2 - offset_2);
            Serial.print(" = ");
            Serial.println(filter_tempC_wJacket->get_physical_value());

            delay(25);
        }
    }
#endif
}

void read_tempC_sensors(bool is_startup_call)
{
    static uint8_t      index = 0;
    static uint16_t     tempC_product_val[9] = { 0 },
                        tempC_wJacket_val[9] = { 0 };
    static uint16_t     tempC_product_val_filtered = is_startup_call ? AnIn_state[ADC_TEMPC_PRODUCT] : tempC_product_val_filtered,
                        tempC_wJacket_val_filtered = is_startup_call ? AnIn_state[ADC_TEMPC_WJACKET] : tempC_wJacket_val_filtered;

#ifdef DEMO_WITHOUT_EXTERNAL_POWER
    result_1 = 1250;
    result_2 = 1250;
#else
    result_1 = STM32->get(COMM_GET::ANIN, ADC_TEMPC_PRODUCT);
    result_2 = STM32->get(COMM_GET::ANIN, ADC_TEMPC_WJACKET);
#endif

    tempC_product_val[index = index >= 9 ? 0 : index]
        = result_1 >= offset_1 ? result_1 - offset_1 : result_1;

#ifdef IS_SSPS3F1_BLACKOUT_EDITION
    static float final_ofs = 0;

    final_ofs = prog_runned.local().is_runned ? offset_prog : offset_2;

    tempC_wJacket_val[index++]
        = result_2 >= result_2 ? result_2 - final_ofs : result_2;
#else
    tempC_wJacket_val[index++]
        = result_2 >= result_2 ? result_2 - offset_2 : result_2;
#endif

    if (index >= 9)
    {
        std::sort(tempC_product_val, tempC_product_val + 9);
        std::sort(tempC_wJacket_val, tempC_wJacket_val + 9);

        AnIn_state[ADC_TEMPC_PRODUCT] = (tempC_product_val[3] + tempC_product_val[4] + tempC_product_val[5]) / 3 - 0;
        AnIn_state[ADC_TEMPC_WJACKET] = (tempC_wJacket_val[3] + tempC_wJacket_val[4] + tempC_wJacket_val[5]) / 3 - 6;

        exp_filter_tempC_product.add_value(AnIn_state[ADC_TEMPC_PRODUCT]);
        exp_filter_tempC_wJacket.add_value(AnIn_state[ADC_TEMPC_WJACKET]);

        if (false)
        {
        Serial.print("s1: ");
        Serial.print(AnIn_state[ADC_TEMPC_PRODUCT]);
        Serial.print(" => ");
        Serial.print(filter_tempC_product->get_physical_value());
        Serial.println(" *c;");
    
        Serial.print("s2: ");
        Serial.print(AnIn_state[ADC_TEMPC_WJACKET]);
        Serial.print(" => ");
        Serial.print(filter_tempC_wJacket->get_physical_value());
        Serial.println(" *c;");

        Serial.println("");
        }
    }
}