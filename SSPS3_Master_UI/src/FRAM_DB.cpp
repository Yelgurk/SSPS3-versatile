#include "FRAM_DB.hpp"

static TMPEProgramTemplate def_prog_tmpe_full = TMPEProgramTemplate(
    ProgramStep(ProgramStepAimEnum::PASTEUR,    0, 20, 65, 1800, 0, 0, 0, 1),
    ProgramStep(ProgramStepAimEnum::CHILLING,   1, 30, 42, 0,    0, 1, 0, 1),
    ProgramStep(ProgramStepAimEnum::HEATING,    2, 10, 42, 60,   0, 0, 1, 0)
);

static TMPEProgramTemplate def_prog_tmpe_heating = TMPEProgramTemplate(
    ProgramStep(ProgramStepAimEnum::PASTEUR,    0, 0,  0,  0,   0, 0, 0, 0),
    ProgramStep(ProgramStepAimEnum::CHILLING,   1, 0,  0,  0,   0, 0, 0, 0),
    ProgramStep(ProgramStepAimEnum::HEATING,    2, 10, 42, 0,   0, 0, 1, 1)
);

static TMPEProgramTemplate def_prog_tmpe_chilling = TMPEProgramTemplate(
    ProgramStep(ProgramStepAimEnum::PASTEUR,    0, 0,  0,  0,   0, 0, 0, 0),
    ProgramStep(ProgramStepAimEnum::CHILLING,   1, 30, 42, 0,   0, 1, 0, 1),
    ProgramStep(ProgramStepAimEnum::HEATING,    2, 0,  0,  0,   0, 0, 0, 0)
);

static S_Time def_wd_boot_time_1 = S_Time(8, 0, 0);
static S_Time def_wd_boot_time_2 = S_Time(14, 0, 0);
static S_Time def_wd_boot_time_3 = S_Time(23, 0, 0);

static CHMProgramTemplate def_prog_chm_null = CHMProgramTemplate(
    ProgramStep(ProgramStepAimEnum::PASTEUR, 0, 0, 0, 0, 0, 0, 0, 0),
    ProgramStep(ProgramStepAimEnum::CHILLING, 1, 0, 0, 0, 0, 0, 0, 0),
    ProgramStep(ProgramStepAimEnum::CUTTING, 2, 0, 0, 0, 0, 0, 0, 0),
    ProgramStep(ProgramStepAimEnum::MIXING, 3, 0, 0, 0, 0, 0, 0, 0),
    ProgramStep(ProgramStepAimEnum::HEATING, 4, 0, 0, 0, 0, 0, 0, 0),
    ProgramStep(ProgramStepAimEnum::DRYING, 5, 0, 0, 0, 0, 0, 0, 0)
);

static CHMProgramTemplate def_prog_chm_adygeiskij = CHMProgramTemplate(
    ProgramStep(ProgramStepAimEnum::PASTEUR,  0, 20, 90, 240, 0, 0, 0, 1),
    ProgramStep(ProgramStepAimEnum::CHILLING, 1, 0,  0,  0, 0, 0, 0, 0),
    ProgramStep(ProgramStepAimEnum::CUTTING,  2, 0,  0,  0, 0, 0, 0, 0),
    ProgramStep(ProgramStepAimEnum::MIXING,   3, 0,  0,  0, 0, 0, 0, 0),
    ProgramStep(ProgramStepAimEnum::HEATING,  4, 0,  90, 240, 0, 0, 0, 1),
    ProgramStep(ProgramStepAimEnum::DRYING,   5, 0,  0,  0, 0, 0, 0, 0)
);

static CHMProgramTemplate def_prog_chm_cottage_cheese = CHMProgramTemplate(
    ProgramStep(ProgramStepAimEnum::PASTEUR,  0, 20, 90, 240, 0, 0, 0, 1),
    ProgramStep(ProgramStepAimEnum::CHILLING, 1, 20, 50, 240, 0, 0, 0, 1),
    ProgramStep(ProgramStepAimEnum::CUTTING,  2, 0,  0,  0, 0, 0, 0, 0),
    ProgramStep(ProgramStepAimEnum::MIXING,   3, 0,  0,  0, 0, 0, 0, 0),
    ProgramStep(ProgramStepAimEnum::HEATING,  4, 0,  0,  0, 0, 0, 0, 0),
    ProgramStep(ProgramStepAimEnum::DRYING,   5, 0,  0,  0, 0, 0, 0, 0)
);

static CHMProgramTemplate def_prog_chm_yogurt = CHMProgramTemplate(
    ProgramStep(ProgramStepAimEnum::PASTEUR,  0, 20, 90, 240, 0, 0, 0, 1),
    ProgramStep(ProgramStepAimEnum::CHILLING, 1, 6,  50, 0, 0, 0, 0, 1),
    ProgramStep(ProgramStepAimEnum::CUTTING,  2, 0,  0,  0, 0, 0, 0, 0),
    ProgramStep(ProgramStepAimEnum::MIXING,   3, 0,  0,  0, 0, 0, 0, 0),
    ProgramStep(ProgramStepAimEnum::HEATING,  4, 0,  50, 14400, 0, 0, 0, 1),
    ProgramStep(ProgramStepAimEnum::DRYING,   5, 0,  15, 0, 0, 0, 0, 1)
);

/*
static CHMProgramTemplate def_prog_chm_parmezan = CHMProgramTemplate(
    ProgramStep(ProgramStepAimEnum::PASTEUR,  0, 5,  82, 20,   0, 0, 0, 1),
    ProgramStep(ProgramStepAimEnum::CHILLING, 1, 10, 32, 1800, 0, 1, 1, 1),
    ProgramStep(ProgramStepAimEnum::CUTTING,  2, 3,  32, 300,  0, 0, 0, 1),
    ProgramStep(ProgramStepAimEnum::MIXING,   3, 15, 32, 2400, 0, 0, 1, 1),
    ProgramStep(ProgramStepAimEnum::HEATING,  4, 25, 50, 1200, 0, 0, 0, 1),
    ProgramStep(ProgramStepAimEnum::DRYING,   5, 30, 50, 1800, 0, 0, 0, 1)
);

static CHMProgramTemplate def_prog_chm_tylzickij = CHMProgramTemplate(
    ProgramStep(ProgramStepAimEnum::PASTEUR,  0, 5,  72, 20,   0, 0, 0, 1),
    ProgramStep(ProgramStepAimEnum::CHILLING, 1, 15, 32, 1800, 0, 1, 1, 1),
    ProgramStep(ProgramStepAimEnum::CUTTING,  2, 3,  32, 300,  0, 0, 0, 1),
    ProgramStep(ProgramStepAimEnum::MIXING,   3, 15, 32, 2400, 0, 0, 1, 1),
    ProgramStep(ProgramStepAimEnum::HEATING,  4, 25, 50, 1200, 0, 0, 0, 1),
    ProgramStep(ProgramStepAimEnum::DRYING,   5, 30, 50, 2580, 0, 0, 0, 1)
);
*/

static ProgramControl def_prog_runned_null = ProgramControl();
static ProgramStep def_prog_runned_step_null = ProgramStep();

// Vars init in FRAM using Storage::allocate
FRAMObject<std::string>& var_rights_key                         = Storage::allocate<std::string>(prog_rights, true, ALLOC_STARTUP_KEY, 40, "plc_startup_key");
FRAMObject<uint32_t>& var_startup_key                           = Storage::allocate<uint32_t>(startup_key, true);

// last rtc clock
FRAMObject<S_DateTime>& var_last_rt                             = Storage::allocate<S_DateTime>(S_DateTime(), true, ALLOC_SYS_VAR_BEGIN);

// master settings - page 1
FRAMObject<EquipmentType>& var_type_of_equipment_enum           = Storage::allocate<EquipmentType>(EquipmentType::DairyTaxiPasteurizerFlowgun, false, ALLOC_SETTINGS_MASTER);
FRAMObject<bool>&   var_stop_btn_type                           = Storage::allocate<bool>(false, false);
FRAMObject<bool>&   var_is_asyncM_rpm_float                     = Storage::allocate<bool>(false, false);
FRAMObject<uint8_t>& var_plc_language                           = Storage::allocate<uint8_t>(0, false);
FRAMObject<bool>& var_equip_have_wJacket_tempC_sensor           = Storage::allocate<bool>(false, false);

// master settings - page 2
FRAMObject<float>& var_sensor_batt_min_V                        = Storage::allocate<float>(MIN_BATT_VOLTAGE, false);   // ok
FRAMObject<float>& var_sensor_batt_max_V                        = Storage::allocate<float>(MAX_BATT_VOLTAGE, false);                           // ok
FRAMObject<uint16_t>& var_sensor_batt_V_min_12bit               = Storage::allocate<uint16_t>(BATT_V_TO_12BIT * MIN_BATT_VOLTAGE, false);      // ok
FRAMObject<uint16_t>& var_sensor_batt_V_max_12bit               = Storage::allocate<uint16_t>(BATT_V_TO_12BIT * MAX_BATT_VOLTAGE, false);      // ok
FRAMObject<uint16_t>& var_sensor_tempC_limit_4ma_12bit          = Storage::allocate<uint16_t>(MIN_ADC_4ma, false);     // ok
FRAMObject<uint16_t>& var_sensor_tempC_limit_20ma_12bit         = Storage::allocate<uint16_t>(MAX_ADC_20ma, false);    // ok
FRAMObject<int16_t>& var_sensor_tempC_limit_4ma_degrees_C       = Storage::allocate<int16_t>(-50, false);              // ok
FRAMObject<int16_t>& var_sensor_tempC_limit_20ma_degrees_C      = Storage::allocate<int16_t>(150, false);              // ok
FRAMObject<uint16_t>& var_sensor_dac_rpm_limit_min_12bit        = Storage::allocate<uint16_t>(0, false);               // nowhere useful
FRAMObject<uint16_t>& var_sensor_dac_rpm_limit_max_12bit        = Storage::allocate<uint16_t>(BIT_12_VAL / 2, false);  // ok, P.S. 4096 = 10V, but out default async driver has max 5V signal, this why (4095 / 2) like (10v / 2)
FRAMObject<uint8_t>& var_sensor_dac_asyncM_rpm_min              = Storage::allocate<uint8_t>(0, false);                // nowhere useful
FRAMObject<uint8_t>& var_sensor_dac_asyncM_rpm_max              = Storage::allocate<uint8_t>(30, false);               // ok
 
// master settings - page 3
FRAMObject<uint8_t>& var_blowing_await_ss                       = Storage::allocate<uint8_t>(2, false);  // ok
FRAMObject<float>& var_blowing_pump_power_lm                    = Storage::allocate<float>(36, false);                         // ok
FRAMObject<uint16_t>& var_blowing_limit_ml_max                  = Storage::allocate<uint16_t>(5000, false);                    // в настройки юзера лимиты
FRAMObject<uint16_t>& var_blowing_limit_ml_min                  = Storage::allocate<uint16_t>(250, false);                     // в настройки юзера лимиты
FRAMObject<uint16_t>& var_blowing_limit_ss_max                  = Storage::allocate<uint16_t>(600, false);                     // в настройки юзера лимиты
FRAMObject<uint16_t>& var_blowing_limit_ss_min                  = Storage::allocate<uint16_t>(10, false);                      // в настройки юзера лимиты

// master settings - page 4
FRAMObject<uint8_t>& var_wJacket_tempC_limit_max                = Storage::allocate<uint8_t>(LIMIT_WATER_BOILING_POINT_TEMPC, false);  // ok
FRAMObject<uint8_t>& var_prog_wJacket_drain_max_ss              = Storage::allocate<uint8_t>(30, false);                               // ok
FRAMObject<uint16_t>& var_prog_on_pause_max_await_ss            = Storage::allocate<uint16_t>(3600, false);                            // ok
FRAMObject<uint16_t>& var_prog_await_spite_of_already_runned_ss = Storage::allocate<uint16_t>(600, false);                             // ok
FRAMObject<uint8_t>& var_prog_limit_heat_tempC_max              = Storage::allocate<uint8_t>(LIMIT_WATER_BOILING_POINT_TEMPC - 3, false);  // ok 
FRAMObject<uint8_t>& var_prog_limit_heat_tempC_min              = Storage::allocate<uint8_t>(30, false);                               // ok
FRAMObject<uint8_t>& var_prog_limit_chill_tempC_max             = Storage::allocate<uint8_t>(50, false);                               // ok
FRAMObject<uint8_t>& var_prog_limit_chill_tempC_min             = Storage::allocate<uint8_t>(5, false);                                // ok
FRAMObject<uint16_t>& var_prog_any_step_max_durat_ss            = Storage::allocate<uint16_t>(3600 * 5, false);                        // ok
FRAMObject<bool>& var_prog_coolign_water_safe_mode              = Storage::allocate<bool>(false, false);                               // ok
FRAMObject<uint8_t>& var_prog_heaters_toggle_delay_ss           = Storage::allocate<uint8_t>(20, false);                               // ok
FRAMObject<uint8_t>& var_prog_wJacket_toggle_delay_ss           = Storage::allocate<uint8_t>(10, false);                               // ok

// user settings - page 1
FRAMObject<S_DateTime>& var_rt_setter                           = Storage::allocate<S_DateTime>(S_DateTime(), true, ALLOC_SETTINGS_USER);

// user settings - page 2
FRAMObject<float>& var_blow_pump_calibration_lm                 = Storage::allocate<float>(0.f);
FRAMObject<BlowgunValue> var_blowiong_val_ml_1                  = Storage::allocate<BlowgunValue>(BlowgunValue(5000));
FRAMObject<BlowgunValue> var_blowiong_val_ml_2                  = Storage::allocate<BlowgunValue>(BlowgunValue(5000));
FRAMObject<BlowgunValue> var_blowiong_val_ml_3                  = Storage::allocate<BlowgunValue>(BlowgunValue(5000));
FRAMObject<BlowgunValue> var_blowiong_val_timer                 = Storage::allocate<BlowgunValue>(BlowgunValue(true, 180));

// user settings - another pages
FRAMObject<TMPEProgramTemplate>& prog_tmpe_main                 = Storage::allocate<TMPEProgramTemplate>(def_prog_tmpe_full, true, ALLOC_TEMPLATES_BEGIN);
FRAMObject<TMPEProgramTemplate>& prog_tmpe_cooling              = Storage::allocate<TMPEProgramTemplate>(def_prog_tmpe_chilling);
FRAMObject<TMPEProgramTemplate>& prog_tmpe_heating              = Storage::allocate<TMPEProgramTemplate>(def_prog_tmpe_heating);
FRAMObject<TMPEProgramTemplate>& prog_tmpe_wd_1                 = Storage::allocate<TMPEProgramTemplate>(def_prog_tmpe_full);
FRAMObject<TMPEProgramTemplate>& prog_tmpe_wd_2                 = Storage::allocate<TMPEProgramTemplate>(def_prog_tmpe_full);
FRAMObject<TMPEProgramTemplate>& prog_tmpe_wd_3                 = Storage::allocate<TMPEProgramTemplate>(def_prog_tmpe_full);
FRAMObject<S_Time>& prog_tmpe_wd_1_boot_time                    = Storage::allocate<S_Time>(def_wd_boot_time_1);                   
FRAMObject<S_Time>& prog_tmpe_wd_2_boot_time                    = Storage::allocate<S_Time>(def_wd_boot_time_2);
FRAMObject<S_Time>& prog_tmpe_wd_3_boot_time                    = Storage::allocate<S_Time>(def_wd_boot_time_3);
FRAMObject<AutoProgStates>& prog_tmpe_wd_1_on_off               = Storage::allocate<AutoProgStates>(AutoProgStates());
FRAMObject<AutoProgStates>& prog_tmpe_wd_2_on_off               = Storage::allocate<AutoProgStates>(AutoProgStates());
FRAMObject<AutoProgStates>& prog_tmpe_wd_3_on_off               = Storage::allocate<AutoProgStates>(AutoProgStates());

FRAMObject<CHMProgramTemplate>& prog_chm_templ_1                = Storage::allocate<CHMProgramTemplate>(def_prog_chm_adygeiskij);
FRAMObject<CHMProgramTemplate>& prog_chm_templ_2                = Storage::allocate<CHMProgramTemplate>(def_prog_chm_cottage_cheese);
FRAMObject<CHMProgramTemplate>& prog_chm_templ_3                = Storage::allocate<CHMProgramTemplate>(def_prog_chm_yogurt);
FRAMObject<CHMProgramTemplate>& prog_chm_templ_4                = Storage::allocate<CHMProgramTemplate>(def_prog_chm_null);
FRAMObject<CHMProgramTemplate>& prog_chm_templ_5                = Storage::allocate<CHMProgramTemplate>(def_prog_chm_null);
FRAMObject<CHMProgramTemplate>& prog_chm_templ_6                = Storage::allocate<CHMProgramTemplate>(def_prog_chm_null);
FRAMObject<CHMProgramTemplate>& prog_chm_templ_7                = Storage::allocate<CHMProgramTemplate>(def_prog_chm_null);
FRAMObject<CHMProgramTemplate>& prog_chm_templ_8                = Storage::allocate<CHMProgramTemplate>(def_prog_chm_null);
FRAMObject<CHMProgramTemplate>& prog_chm_templ_9                = Storage::allocate<CHMProgramTemplate>(def_prog_chm_null);
FRAMObject<CHMProgramTemplate>& prog_chm_templ_10               = Storage::allocate<CHMProgramTemplate>(def_prog_chm_null);

// private vars
FRAMObject<uint16_t>& prog_runned_steps_count                   = Storage::allocate<uint16_t>(0);
FRAMObject<uint8_t>& prog_active_step                           = Storage::allocate<uint8_t>(0);
FRAMObject<uint8_t>& prog_next_step                             = Storage::allocate<uint8_t>(0);
FRAMObject<ProgramControl>& prog_runned                         = Storage::allocate<ProgramControl>(def_prog_runned_null);
FRAMObject<ProgramStep>& prog_runned_step_1                     = Storage::allocate<ProgramStep>(def_prog_runned_step_null);
FRAMObject<ProgramStep>& prog_runned_step_2                     = Storage::allocate<ProgramStep>(def_prog_runned_step_null);
FRAMObject<ProgramStep>& prog_runned_step_3                     = Storage::allocate<ProgramStep>(def_prog_runned_step_null);
FRAMObject<ProgramStep>& prog_runned_step_4                     = Storage::allocate<ProgramStep>(def_prog_runned_step_null);
FRAMObject<ProgramStep>& prog_runned_step_5                     = Storage::allocate<ProgramStep>(def_prog_runned_step_null);
FRAMObject<ProgramStep>& prog_runned_step_6                     = Storage::allocate<ProgramStep>(def_prog_runned_step_null);
FRAMObject<ProgramStep>& prog_runned_step_7                     = Storage::allocate<ProgramStep>(def_prog_runned_step_null);
FRAMObject<ProgramStep>& prog_runned_step_8                     = Storage::allocate<ProgramStep>(def_prog_runned_step_null);
FRAMObject<ProgramStep>& prog_runned_step_9                     = Storage::allocate<ProgramStep>(def_prog_runned_step_null);
FRAMObject<ProgramStep>& prog_runned_step_10                    = Storage::allocate<ProgramStep>(def_prog_runned_step_null);
FRAMObject<ProgramStep>& prog_runned_step_11                    = Storage::allocate<ProgramStep>(def_prog_runned_step_null);
FRAMObject<ProgramStep>& prog_runned_step_12                    = Storage::allocate<ProgramStep>(def_prog_runned_step_null);
FRAMObject<ProgramStep>& prog_runned_step_13                    = Storage::allocate<ProgramStep>(def_prog_runned_step_null);
FRAMObject<ProgramStep>& prog_runned_step_14                    = Storage::allocate<ProgramStep>(def_prog_runned_step_null);
FRAMObject<ProgramStep>& prog_runned_step_15                    = Storage::allocate<ProgramStep>(def_prog_runned_step_null);
FRAMObject<ProgramStep>& prog_runned_step_16                    = Storage::allocate<ProgramStep>(def_prog_runned_step_null);
FRAMObject<ProgramStep>& prog_runned_step_17                    = Storage::allocate<ProgramStep>(def_prog_runned_step_null);
FRAMObject<ProgramStep>& prog_runned_step_18                    = Storage::allocate<ProgramStep>(def_prog_runned_step_null);
FRAMObject<ProgramStep>& prog_runned_step_19                    = Storage::allocate<ProgramStep>(def_prog_runned_step_null);
FRAMObject<ProgramStep>& prog_runned_step_20                    = Storage::allocate<ProgramStep>(def_prog_runned_step_null);
FRAMObject<ProgramStep>& prog_runned_step_21                    = Storage::allocate<ProgramStep>(def_prog_runned_step_null);
FRAMObject<ProgramStep>& prog_runned_step_22                    = Storage::allocate<ProgramStep>(def_prog_runned_step_null);
FRAMObject<ProgramStep>& prog_runned_step_23                    = Storage::allocate<ProgramStep>(def_prog_runned_step_null);
FRAMObject<ProgramStep>& prog_runned_step_24                    = Storage::allocate<ProgramStep>(def_prog_runned_step_null);

// time x func
FRAMObject<uint32_t>& prod_time_x_cnt                           = Storage::allocate<uint32_t>(0, false);

vector<FRAMObject<TMPEProgramTemplate>*> * prog_tmpe_templates = new vector<FRAMObject<TMPEProgramTemplate>*>{
    &prog_tmpe_main,
    &prog_tmpe_cooling,
    &prog_tmpe_heating,
    &prog_tmpe_wd_1,
    &prog_tmpe_wd_2,
    &prog_tmpe_wd_3
};

vector<FRAMObject<S_Time>*> * prog_tmpe_templates_wd_time = new vector<FRAMObject<S_Time>*>{
    &prog_tmpe_wd_1_boot_time,
    &prog_tmpe_wd_2_boot_time,
    &prog_tmpe_wd_3_boot_time
};

vector<FRAMObject<AutoProgStates>*> * prog_tmpe_templates_wd_state = new vector<FRAMObject<AutoProgStates>*>{
    &prog_tmpe_wd_1_on_off,
    &prog_tmpe_wd_2_on_off,
    &prog_tmpe_wd_3_on_off
};

vector<FRAMObject<CHMProgramTemplate>*> * prog_chm_templates = new vector<FRAMObject<CHMProgramTemplate>*>{
    &prog_chm_templ_1,
    &prog_chm_templ_2,
    &prog_chm_templ_3,
    &prog_chm_templ_4,
    &prog_chm_templ_5,
    &prog_chm_templ_6,
    &prog_chm_templ_7,
    &prog_chm_templ_8,
    &prog_chm_templ_9,
    &prog_chm_templ_10
};

vector<FRAMObject<ProgramStep>*> * prog_runned_steps = new vector<FRAMObject<ProgramStep>*>{
    &prog_runned_step_1,
    &prog_runned_step_2,
    &prog_runned_step_3,
    &prog_runned_step_4,
    &prog_runned_step_5,
    &prog_runned_step_6,
    &prog_runned_step_7,
    &prog_runned_step_8,
    &prog_runned_step_9,
    &prog_runned_step_10,
    &prog_runned_step_11,
    &prog_runned_step_12,
    &prog_runned_step_13,
    &prog_runned_step_14,
    &prog_runned_step_15,
    &prog_runned_step_16,
    &prog_runned_step_17,
    &prog_runned_step_18,
    &prog_runned_step_19,
    &prog_runned_step_20,
    &prog_runned_step_21,
    &prog_runned_step_22,
    &prog_runned_step_23,
    &prog_runned_step_24
};

vector<FRAMObject<BlowgunValue>*> * blowing_vals = new vector<FRAMObject<BlowgunValue>*> {
    &var_blowiong_val_ml_1,
    &var_blowiong_val_ml_2,
    &var_blowiong_val_ml_3,
    &var_blowiong_val_timer
};