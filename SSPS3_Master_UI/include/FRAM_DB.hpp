#pragma once

#ifndef FRAM_DB_hpp
#define FRAM_DB_hpp

#include <Arduino.h>
#include "ProgramControl.hpp"
#include "BlowingControl.hpp"
#include "../../SSPS3_Master_Domain/include/FRAM/FRAM_Storage.hpp"

/*
static auto& mem_Timer1 = Storage::allocate<bool>(false);
static auto& mem_Timer2 = Storage::allocate<uint8_t>(255);
static auto& mem_Timer3 = Storage::allocate<uint16_t>(255);
static auto& mem_Timer4 = Storage::allocate<uint32_t>(255);
static auto& mem_DT = Storage::allocate<S_DateTime>(defaultDT);
static auto& mem_TDS_3 = Storage::allocate<TaskDataStruct>(defaultTaskData);
static auto& mem_String = Storage::allocate<std::string>("err");
*/



#define ALLOC_SYS_VAR_BEGIN     0
#define ALLOC_SENS_VAR_BEGIN    500
#define ALLOC_CONF_VAR_BEGIN    1000
#define ALLOC_USER_VAR_BEGIN    2000

struct __attribute__((packed)) CheesemakerProgramTemplate
{
    
    
    CheesemakerProgramTemplate() {}     
};

static CheesemakerProgramTemplate Cheesemaker_conf_default = CheesemakerProgramTemplate();

/*
static auto& var_type_of_equipment_enum                = Storage::allocate<uint8_t>(0, ALLOC_SYS_VAR_BEGIN);
static auto& var_is_blowgun_by_rf                      = Storage::allocate<bool>(false);

static auto& var_sensor_voltage_min_12bit              = Storage::allocate<uint16_t>(BATT_VOLTAGE_TO_12BIT(22.8f), ALLOC_SENS_VAR_BEGIN);
static auto& var_sensor_voltage_max_12bit              = Storage::allocate<uint16_t>(BATT_VOLTAGE_TO_12BIT(25.f));
static auto& var_sensor_tempC_limit_4ma_12bit          = Storage::allocate<uint16_t>(373);
static auto& var_sensor_tempC_limit_20ma_12bit         = Storage::allocate<uint16_t>(1865);
static auto& var_sensor_tempC_limit_4ma_degrees_C      = Storage::allocate<int16_t>(-50);
static auto& var_sensor_tempC_limit_20ma_degrees_C     = Storage::allocate<int16_t>(150);
static auto& var_wJacket_tempC_limit_max               = Storage::allocate<uint8_t>(93);

static auto& var_blowing_await_ss                      = Storage::allocate<uint8_t>(2, ALLOC_CONF_VAR_BEGIN);
static auto& var_blowing_pump_power_lm                 = Storage::allocate<float>(36);
static auto& var_blowing_limit_ml_max                  = Storage::allocate<uint16_t>(5000);
static auto& var_blowing_limit_ml_min                  = Storage::allocate<uint16_t>(250);
static auto& var_blowing_limit_ss_max                  = Storage::allocate<uint16_t>(600);
static auto& var_blowing_limit_ss_min                  = Storage::allocate<uint16_t>(10);
static auto& var_prog_wJacket_drain_max_ss             = Storage::allocate<uint8_t>(30);
static auto& var_prog_on_pause_max_await_ss            = Storage::allocate<uint16_t>(3600);
static auto& var_prog_await_spite_of_cancelation_ss    = Storage::allocate<uint16_t>(600);
static auto& var_prog_limit_heat_tempC_max             = Storage::allocate<uint8_t>(92);
static auto& var_prog_limit_heat_tempC_min             = Storage::allocate<uint8_t>(30);
static auto& var_prog_limit_chill_tempC_max            = Storage::allocate<uint8_t>(50);
static auto& var_prog_limit_chill_tempC_min            = Storage::allocate<uint8_t>(5);
static auto& var_prog_any_step_max_durat_ss            = Storage::allocate<uint16_t>(3600);
static auto& var_prog_any_step_min_durat_ss            = Storage::allocate<uint16_t>(0);
static auto& var_prog_heaters_toggle_delay_ss          = Storage::allocate<uint8_t>(20);
static auto& var_prog_wJacket_toggle_delay_ss          = Storage::allocate<uint8_t>(10);

static auto& var_tmp_default_tempC_pasteur             = Storage::allocate<uint8_t>(85, ALLOC_USER_VAR_BEGIN);
static auto& var_tmp_default_tempC_chill               = Storage::allocate<uint8_t>(25);
static auto& var_tmp_default_tempC_heat                = Storage::allocate<uint8_t>(50);
static auto& var_tmp_default_durat_ss_pasteur          = Storage::allocate<uint16_t>(600);
static auto& var_tmp_default_durat_ss_chill            = Storage::allocate<uint16_t>(300);
static auto& var_tmp_default_durat_ss_heat             = Storage::allocate<uint16_t>(120);
static auto& var_tmp_default_heat_untill_user_stop     = Storage::allocate<boolean>(false);

static auto& var_tmp_heatProg_tempC_heat               = Storage::allocate<uint8_t>(50);
static auto& var_tmp_heatProg_durat_ss_heat            = Storage::allocate<uint16_t>(120);
static auto& var_tmp_heatProg_heat_untill_user_stop    = Storage::allocate<boolean>(false);

static auto& var_tmp_chillProg_tempC_chill             = Storage::allocate<uint8_t>(25);
static auto& var_tmp_chillProg_durat_ss_chill          = Storage::allocate<uint16_t>(300);

static auto& var_tmp_watchdog_1_tempC_pasteur          = Storage::allocate<uint8_t>(85);
static auto& var_tmp_watchdog_1_tempC_chill            = Storage::allocate<uint8_t>(25);
static auto& var_tmp_watchdog_1_tempC_heat             = Storage::allocate<uint8_t>(50);
static auto& var_tmp_watchdog_1_durat_ss_pasteur       = Storage::allocate<uint16_t>(600);
static auto& var_tmp_watchdog_1_durat_ss_chill         = Storage::allocate<uint16_t>(300);
static auto& var_tmp_watchdog_1_durat_ss_heat          = Storage::allocate<uint16_t>(120);
static auto& var_tmp_watchdog_1_heat_untill_user_stop  = Storage::allocate<boolean>(false);
static auto& var_tmp_watchdog_1_is_active              = Storage::allocate<boolean>(false);

static auto& var_tmp_watchdog_2_tempC_pasteur          = Storage::allocate<uint8_t>(85);
static auto& var_tmp_watchdog_2_tempC_chill            = Storage::allocate<uint8_t>(25);
static auto& var_tmp_watchdog_2_tempC_heat             = Storage::allocate<uint8_t>(50);
static auto& var_tmp_watchdog_2_durat_ss_pasteur       = Storage::allocate<uint16_t>(600);
static auto& var_tmp_watchdog_2_durat_ss_chill         = Storage::allocate<uint16_t>(300);
static auto& var_tmp_watchdog_2_durat_ss_heat          = Storage::allocate<uint16_t>(120);
static auto& var_tmp_watchdog_2_heat_untill_user_stop  = Storage::allocate<boolean>(false);
static auto& var_tmp_watchdog_2_is_active              = Storage::allocate<boolean>(false);

static auto& var_tmp_watchdog_3_tempC_pasteur          = Storage::allocate<uint8_t>(85);
static auto& var_tmp_watchdog_3_tempC_chill            = Storage::allocate<uint8_t>(25);
static auto& var_tmp_watchdog_3_tempC_heat             = Storage::allocate<uint8_t>(50);
static auto& var_tmp_watchdog_3_durat_ss_pasteur       = Storage::allocate<uint16_t>(600);
static auto& var_tmp_watchdog_3_durat_ss_chill         = Storage::allocate<uint16_t>(300);
static auto& var_tmp_watchdog_3_durat_ss_heat          = Storage::allocate<uint16_t>(120);
static auto& var_tmp_watchdog_3_heat_untill_user_stop  = Storage::allocate<boolean>(false);
static auto& var_tmp_watchdog_3_is_active              = Storage::allocate<boolean>(false);
*/

//static auto& var_chm_default_prog_1                    = Storage::allocate<CheesemakerConf>(Cheesemaker_conf_default);

#endif