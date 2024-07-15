#pragma once

#ifndef FRAM_DB_HPP
#define FRAM_DB_HPP

#include <Arduino.h>
#include "ProgramControl.hpp"
#include "BlowingControl.hpp"
#include "../../SSPS3_Master_Domain/include/FRAM/FRAM_Storage.hpp"
#include "../../SSPS3_Master_Domain/include/FRAM/FRAM_Object.hpp"
#include "../../SSPS3_Master_Domain/include/FRAM/FRAM_RW.hpp"

/*
static auto& mem_Timer1 = Storage::allocate<bool>(false);
static auto& mem_Timer2 = Storage::allocate<uint8_t>(255);
static auto& mem_Timer3 = Storage::allocate<uint16_t>(255);
static auto& mem_Timer4 = Storage::allocate<uint32_t>(255);
static auto& mem_DT = Storage::allocate<S_DateTime>(defaultDT);
static auto& mem_TDS_3 = Storage::allocate<TaskDataStruct>(defaultTaskData);
static auto& mem_String = Storage::allocate<std::string>("err");
*/

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

/* ADC limits */
#define BATT_MULT_TO_12BIT  69.7674f
#define MIN_BATT_VOLTAGE    22.8f
#define MAX_BATT_VOLTAGE    25.f
#define MIN_ADC_4ma         373
#define MAX_ADC_20ma        1865
#define MIN_ADC_TEMPC       -50
#define MAX_ADC_TEMPC       150


#define ALLOC_SYS_VAR_BEGIN     0
#define ALLOC_SENS_VAR_BEGIN    500
#define ALLOC_CONF_VAR_BEGIN    1000
#define ALLOC_USER_VAR_BEGIN    2000

struct __attribute__((packed)) CheesemakerProgramTemplate
{
    
    
    CheesemakerProgramTemplate() {}     
};

// Объявляем переменные как extern
extern FRAMObject<uint8_t>& var_type_of_equipment_enum;
extern FRAMObject<bool>& var_is_blowgun_by_rf;
extern FRAMObject<uint16_t>& var_sensor_voltage_min_12bit;
extern FRAMObject<uint16_t>& var_sensor_voltage_max_12bit;
extern FRAMObject<uint16_t>& var_sensor_tempC_limit_4ma_12bit;
extern FRAMObject<uint16_t>& var_sensor_tempC_limit_20ma_12bit;
extern FRAMObject<int16_t>& var_sensor_tempC_limit_4ma_degrees_C;
extern FRAMObject<int16_t>& var_sensor_tempC_limit_20ma_degrees_C;
extern FRAMObject<uint8_t>& var_wJacket_tempC_limit_max;
extern FRAMObject<uint8_t>& var_blowing_await_ss;
extern FRAMObject<float>& var_blowing_pump_power_lm;
extern FRAMObject<uint16_t>& var_blowing_limit_ml_max;
extern FRAMObject<uint16_t>& var_blowing_limit_ml_min;
extern FRAMObject<uint16_t>& var_blowing_limit_ss_max;
extern FRAMObject<uint16_t>& var_blowing_limit_ss_min;
extern FRAMObject<uint8_t>& var_prog_wJacket_drain_max_ss;
extern FRAMObject<uint16_t>& var_prog_on_pause_max_await_ss;
extern FRAMObject<uint16_t>& var_prog_await_spite_of_cancelation_ss;
extern FRAMObject<uint8_t>& var_prog_limit_heat_tempC_max;
extern FRAMObject<uint8_t>& var_prog_limit_heat_tempC_min;
extern FRAMObject<uint8_t>& var_prog_limit_chill_tempC_max;
extern FRAMObject<uint8_t>& var_prog_limit_chill_tempC_min;
extern FRAMObject<uint16_t>& var_prog_any_step_max_durat_ss;
extern FRAMObject<uint16_t>& var_prog_any_step_min_durat_ss;
extern FRAMObject<uint8_t>& var_prog_heaters_toggle_delay_ss;
extern FRAMObject<uint8_t>& var_prog_wJacket_toggle_delay_ss;
extern FRAMObject<uint8_t>& var_tmp_default_tempC_pasteur;
extern FRAMObject<uint8_t>& var_tmp_default_tempC_chill;
extern FRAMObject<uint8_t>& var_tmp_default_tempC_heat;
extern FRAMObject<uint16_t>& var_tmp_default_durat_ss_pasteur;
extern FRAMObject<uint16_t>& var_tmp_default_durat_ss_chill;
extern FRAMObject<uint16_t>& var_tmp_default_durat_ss_heat;
extern FRAMObject<bool>& var_tmp_default_heat_untill_user_stop;
extern FRAMObject<uint8_t>& var_tmp_heatProg_tempC_heat;
extern FRAMObject<uint16_t>& var_tmp_heatProg_durat_ss_heat;
extern FRAMObject<bool>& var_tmp_heatProg_heat_untill_user_stop;
extern FRAMObject<uint8_t>& var_tmp_chillProg_tempC_chill;
extern FRAMObject<uint16_t>& var_tmp_chillProg_durat_ss_chill;
extern FRAMObject<uint8_t>& var_tmp_watchdog_1_tempC_pasteur;
extern FRAMObject<uint8_t>& var_tmp_watchdog_1_tempC_chill;
extern FRAMObject<uint8_t>& var_tmp_watchdog_1_tempC_heat;
extern FRAMObject<uint16_t>& var_tmp_watchdog_1_durat_ss_pasteur;
extern FRAMObject<uint16_t>& var_tmp_watchdog_1_durat_ss_chill;
extern FRAMObject<uint16_t>& var_tmp_watchdog_1_durat_ss_heat;
extern FRAMObject<bool>& var_tmp_watchdog_1_heat_untill_user_stop;
extern FRAMObject<bool>& var_tmp_watchdog_1_is_active;
extern FRAMObject<uint8_t>& var_tmp_watchdog_2_tempC_pasteur;
extern FRAMObject<uint8_t>& var_tmp_watchdog_2_tempC_chill;
extern FRAMObject<uint8_t>& var_tmp_watchdog_2_tempC_heat;
extern FRAMObject<uint16_t>& var_tmp_watchdog_2_durat_ss_pasteur;
extern FRAMObject<uint16_t>& var_tmp_watchdog_2_durat_ss_chill;
extern FRAMObject<uint16_t>& var_tmp_watchdog_2_durat_ss_heat;
extern FRAMObject<bool>& var_tmp_watchdog_2_heat_untill_user_stop;
extern FRAMObject<bool>& var_tmp_watchdog_2_is_active;
extern FRAMObject<uint8_t>& var_tmp_watchdog_3_tempC_pasteur;
extern FRAMObject<uint8_t>& var_tmp_watchdog_3_tempC_chill;
extern FRAMObject<uint8_t>& var_tmp_watchdog_3_tempC_heat;
extern FRAMObject<uint16_t>& var_tmp_watchdog_3_durat_ss_pasteur;
extern FRAMObject<uint16_t>& var_tmp_watchdog_3_durat_ss_chill;
extern FRAMObject<uint16_t>& var_tmp_watchdog_3_durat_ss_heat;
extern FRAMObject<bool>& var_tmp_watchdog_3_heat_untill_user_stop;
extern FRAMObject<bool>& var_tmp_watchdog_3_is_active;

#endif // FRAM_DB_HPP
