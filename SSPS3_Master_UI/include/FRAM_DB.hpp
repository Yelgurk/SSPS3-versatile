#pragma once

#ifndef FRAM_DB_HPP
#define FRAM_DB_HPP

#include <Arduino.h>
#include <vector>
#include "ProgramControl.hpp"
#include "BlowingControl.hpp"
#include "../../SSPS3_Master_Domain/include/FRAM/FRAM_Storage.hpp"
#include "../../SSPS3_Master_Domain/include/FRAM/FRAM_Object.hpp"
#include "../../SSPS3_Master_Domain/include/FRAM/FRAM_RW.hpp"

using namespace std;

/*
static auto& mem_Timer1 = Storage::allocate<bool>(false);
static auto& mem_Timer2 = Storage::allocate<uint8_t>(255);
static auto& mem_Timer3 = Storage::allocate<uint16_t>(255);
static auto& mem_Timer4 = Storage::allocate<uint32_t>(255);
static auto& mem_DT = Storage::allocate<S_DateTime>(defaultDT);
static auto& mem_TDS_3 = Storage::allocate<TaskDataStruct>(defaultTaskData);
static auto& mem_String = Storage::allocate<std::string>("err");
*/

/* Prog conf defines */
#define PROG_RUNNED_STEPS_NCT_MAX   24

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
#define BIT_12_VAL          4095
#define BATT_V_TO_12BIT     79.5f   //69.76744f
#define MIN_BATT_VOLTAGE    22.8f
#define MAX_BATT_VOLTAGE    25.0f
#define MIN_ADC_4ma         373
#define MAX_ADC_20ma        1865
#define MIN_ADC_TEMPC       -50
#define MAX_ADC_TEMPC       150

/* FRAM addr */
#define ALLOC_SYS_VAR_BEGIN     0
#define ALLOC_SENS_VAR_BEGIN    200
#define ALLOC_CONF_VAR_BEGIN    1000
#define ALLOC_USER_VAR_BEGIN    2000

/* Const physical limitations by company */
#define LIMIT_WATER_BOILING_POINT_TEMPC     90

// Объявляем переменные как extern
extern FRAMObject<S_DateTime>& var_last_rt;

extern FRAMObject<uint8_t>& var_type_of_equipment_enum;
extern FRAMObject<uint8_t>& var_plc_language;
extern FRAMObject<bool>& var_is_blowgun_by_rf;
extern FRAMObject<bool>& var_is_asyncM_rpm_float;

extern FRAMObject<float>& var_sensor_batt_min_V;
extern FRAMObject<float>& var_sensor_batt_max_V;
extern FRAMObject<uint16_t>& var_sensor_batt_V_min_12bit;
extern FRAMObject<uint16_t>& var_sensor_batt_V_max_12bit;
extern FRAMObject<uint16_t>& var_sensor_tempC_limit_4ma_12bit;
extern FRAMObject<uint16_t>& var_sensor_tempC_limit_20ma_12bit;
extern FRAMObject<int16_t>& var_sensor_tempC_limit_4ma_degrees_C;
extern FRAMObject<int16_t>& var_sensor_tempC_limit_20ma_degrees_C;
extern FRAMObject<uint16_t>& var_sensor_dac_rpm_limit_min_12bit;
extern FRAMObject<uint16_t>& var_sensor_dac_rpm_limit_max_12bit;
extern FRAMObject<uint8_t>& var_sensor_dac_asyncM_rpm_min;      
extern FRAMObject<uint8_t>& var_sensor_dac_asyncM_rpm_max;      

extern FRAMObject<uint8_t>& var_wJacket_tempC_limit_max;
extern FRAMObject<uint8_t>& var_blowing_await_ss;
extern FRAMObject<float>& var_blowing_pump_power_lm;
extern FRAMObject<uint16_t>& var_blowing_limit_ml_max;
extern FRAMObject<uint16_t>& var_blowing_limit_ml_min;
extern FRAMObject<uint16_t>& var_blowing_limit_ss_max;
extern FRAMObject<uint16_t>& var_blowing_limit_ss_min;

extern FRAMObject<uint8_t>& var_prog_wJacket_drain_max_ss;
extern FRAMObject<uint16_t>& var_prog_on_pause_max_await_ss;
extern FRAMObject<uint16_t>& var_prog_await_spite_of_already_runned_ss;
extern FRAMObject<uint8_t>& var_prog_limit_heat_tempC_max;
extern FRAMObject<uint8_t>& var_prog_limit_heat_tempC_min;
extern FRAMObject<uint8_t>& var_prog_limit_chill_tempC_max;
extern FRAMObject<uint8_t>& var_prog_limit_chill_tempC_min;
extern FRAMObject<uint16_t>& var_prog_any_step_max_durat_ss;
extern FRAMObject<bool>& var_prog_coolign_water_safe_mode;
extern FRAMObject<uint8_t>& var_prog_heaters_toggle_delay_ss;
extern FRAMObject<uint8_t>& var_prog_wJacket_toggle_delay_ss;

extern FRAMObject<S_DateTime>& var_rt_setter;

extern FRAMObject<float>& var_blow_pump_calibration_lm;

extern FRAMObject<TMPEProgramTemplate>& prog_tmpe_main;
extern FRAMObject<TMPEProgramTemplate>& prog_tmpe_heating;
extern FRAMObject<TMPEProgramTemplate>& prog_tmpe_cooling;
extern FRAMObject<TMPEProgramTemplate>& prog_tmpe_wd_1;
extern FRAMObject<TMPEProgramTemplate>& prog_tmpe_wd_2;
extern FRAMObject<TMPEProgramTemplate>& prog_tmpe_wd_3;
extern FRAMObject<S_Time>& prog_tmpe_wd_1_boot_time;
extern FRAMObject<S_Time>& prog_tmpe_wd_2_boot_time;
extern FRAMObject<S_Time>& prog_tmpe_wd_3_boot_time;
extern FRAMObject<bool>& prog_tmpe_wd_1_on_off;
extern FRAMObject<bool>& prog_tmpe_wd_2_on_off;
extern FRAMObject<bool>& prog_tmpe_wd_3_on_off;

extern FRAMObject<CHMProgramTemplate>& prog_chm_templ_1;
extern FRAMObject<CHMProgramTemplate>& prog_chm_templ_2;
extern FRAMObject<CHMProgramTemplate>& prog_chm_templ_3;
extern FRAMObject<CHMProgramTemplate>& prog_chm_templ_4;
extern FRAMObject<CHMProgramTemplate>& prog_chm_templ_5;
extern FRAMObject<CHMProgramTemplate>& prog_chm_templ_6;
extern FRAMObject<CHMProgramTemplate>& prog_chm_templ_7;
extern FRAMObject<CHMProgramTemplate>& prog_chm_templ_8;
extern FRAMObject<CHMProgramTemplate>& prog_chm_templ_9;
extern FRAMObject<CHMProgramTemplate>& prog_chm_templ_10;

extern FRAMObject<uint16_t>& prog_runned_steps_count;
extern FRAMObject<ProgramControl>& prog_runned;
extern FRAMObject<ProgramStep>& prog_runned_step_1;
extern FRAMObject<ProgramStep>& prog_runned_step_2;
extern FRAMObject<ProgramStep>& prog_runned_step_3;
extern FRAMObject<ProgramStep>& prog_runned_step_4;
extern FRAMObject<ProgramStep>& prog_runned_step_5;
extern FRAMObject<ProgramStep>& prog_runned_step_6;
extern FRAMObject<ProgramStep>& prog_runned_step_7;
extern FRAMObject<ProgramStep>& prog_runned_step_8;
extern FRAMObject<ProgramStep>& prog_runned_step_9;
extern FRAMObject<ProgramStep>& prog_runned_step_10;
extern FRAMObject<ProgramStep>& prog_runned_step_11;
extern FRAMObject<ProgramStep>& prog_runned_step_12;
extern FRAMObject<ProgramStep>& prog_runned_step_13;
extern FRAMObject<ProgramStep>& prog_runned_step_14;
extern FRAMObject<ProgramStep>& prog_runned_step_15;
extern FRAMObject<ProgramStep>& prog_runned_step_16;
extern FRAMObject<ProgramStep>& prog_runned_step_17;
extern FRAMObject<ProgramStep>& prog_runned_step_18;
extern FRAMObject<ProgramStep>& prog_runned_step_19;
extern FRAMObject<ProgramStep>& prog_runned_step_20;
extern FRAMObject<ProgramStep>& prog_runned_step_21;
extern FRAMObject<ProgramStep>& prog_runned_step_22;
extern FRAMObject<ProgramStep>& prog_runned_step_23;
extern FRAMObject<ProgramStep>& prog_runned_step_24;

extern vector<FRAMObject<TMPEProgramTemplate>*>     *prog_tmpe_templates;
extern vector<FRAMObject<S_Time>*>                  *prog_tmpe_templates_wd_time;
extern vector<FRAMObject<bool>*>                    *prog_tmpe_templates_wd_state;
extern vector<FRAMObject<CHMProgramTemplate>*>      *prog_chm_templates;
extern vector<FRAMObject<ProgramStep>*>             *prog_runned_steps;

#endif // FRAM_DB_HPP
