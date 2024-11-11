#pragma once

#ifndef UIService_hpp
#define UIService_hpp

#include "../config/LGFX_SSPS3_V1.hpp"
#include <lvgl.h>
#include <Arduino.h>

#include "UIElement.hpp"
#include "UIControls/UIBlowingControl.hpp"
#include "UIControls/UIBlowValListItem.hpp"
#include "UIControls/UICheapRoadmap.hpp"
#include "UIControls/UIDateTime.hpp"
#include "UIControls/UIMachineStateBar.hpp"
#include "UIControls/UIMenuList.hpp"
#include "UIControls/UIMenuListItem.hpp"
#include "UIControls/UINotificationBar.hpp"
#include "UIControls/UINotifyBar.hpp"
#include "UIControls/UIObject.hpp"
#include "UIControls/UIProgramSelectorControl.hpp"
#include "UIControls/UIProgramSelectorItem.hpp"
#include "UIControls/UIScreen.hpp"
#include "UIControls/UITaskListItem.hpp"
#include "UIControls/UITaskRoadmapList.hpp"
#include "UIControls/UIValueSetter.hpp"

#include "BlowingControl.hpp"
#include "ProgramControl.hpp"
#include "FRAM_DB.hpp"
#include "DS3231.h"
#include "Watchdogs/ProgStartupWatchdog.hpp"
#include "UIManager.hpp"

#define LGFX_USE_V1
#define SCREEN_WIDTH            480U
#define SCREEN_HEIGHT           320U
#define INIT_BUFFER_IN_PSRAM    0
#define TEMPLATES_COUNT_TMPE    6
#define TEMPLATES_COUNT_WD      3
#define TEMPLATES_COUNT_CHM     10

#if INIT_BUFFER_IN_PSRAM == 0
    #define SCREEN_BUFFER           (SCREEN_WIDTH * SCREEN_HEIGHT * LV_COLOR_DEPTH) / 16 / 2    
    static uint8_t lv_buff_1[SCREEN_BUFFER];
    static uint8_t lv_buff_2[SCREEN_BUFFER];
#else
    #define SCREEN_BUFFER           (SCREEN_WIDTH * SCREEN_HEIGHT * LV_COLOR_DEPTH)  
    
    class PSRAMBuffer
    {
    private:
        uint8_t* buffer = nullptr;

    public:
        PSRAMBuffer(size_t size)
        {
            buffer = static_cast<uint8_t*>(ps_malloc(size));
            if (!buffer)
                Serial.println("Failed to allocate buffer in PSRAM");
        }

        ~PSRAMBuffer()
        {
            if (buffer)
                free(buffer);
        }

        uint8_t* getBuffer() {
            return buffer;
        }
    };
 
    static PSRAMBuffer * lv_buff_1;
    static PSRAMBuffer * lv_buff_2;
#endif

static LGFX lcd;

static uint32_t arduino_tick_get_cb(void) {
    return millis();
}

static void lcd_flush_cb(lv_display_t* display, const lv_area_t* area, unsigned char* data)
{
    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);
    lv_draw_sw_rgb565_swap(data, w*h);
    lcd.pushImage(area->x1, area->y1, w, h, (uint16_t*)data);
    lv_display_flush_ready(display);
}

extern ProgStartupWatchdog  * prog_stasrtup_wd;
extern BlowingControl       * Blowing_control;
extern DS3231               * rtc;
extern S_DateTime           * dt_rt;
extern UIManager            * UI_manager;

template<typename T>
class ValueComparator
{
public:
    static T calc(const T& min, const T& max, const T& current)
    {
        if (current > max)
            return max;
        else if (current < min)
            return min;
        else
            return current;
    }
};

class UIService
{
private:
    bool is_chm     = false;
    bool is_pasteur = false;
    bool is_blowgun = false;

public:
    /* Display */
    lv_obj_t* screen;

    /* Screens */
    UIMachineStateBar * UI_machine_state_bar;
    UIDateTime * UI_date_time;
    UINotifyBar * UI_notify_bar;
    UINotificationBar * UI_notification_bar;
    UIProgramSelectorControl * UI_prog_selector_control;
    UITaskRoadmapList * UI_task_roadmap_control;
    UICheapRoadmap * UI_cheap_roadmap_control;
    
    UIBlowingControl * UI_blowing_control;

    /* Prog and blowing buff lists */
    vector<UITaskListItem> * UI_task_steps;
    vector<UIBlowValListItem*> Blow_vars;
    vector<UIProgramSelectorItem*> UI_program_selector_items;

    /* Settings - user */
    UIMenuList * UI_menu_list_user;
    UIMenuListItem * UI_settings_rt;
    UIMenuListItem * UI_settings_pump_calibr;
    UIValueSetter * UI_setter_hh;
    UIValueSetter * UI_setter_mm;
    UIValueSetter * UI_setter_ss;
    UIValueSetter * UI_setter_dd;
    UIValueSetter * UI_setter_MM;
    UIValueSetter * UI_setter_yyyy;
    UIValueSetter * UI_rt_setter_accept;
    UIValueSetter * UI_setter_pump_calibr_lm;

    vector<UIMenuListItem*> UI_template_menu_items;
    vector<UIValueSetter*> UI_template_setters;
    uint8_t menu_tmpe_local_start_at    = 0,
            menu_tmpe_general_start_at  = 0,
            menu_wd_local_start_at      = 0,
            menu_wd_general_start_at    = 0,
            menu_chm_local_start_at     = 0,
            menu_chm_general_start_at   = 0;
    UIValueSetter * UI_setter_wd_turn_on_off;
    UIValueSetter * UI_setter_wd_hh;
    UIValueSetter * UI_setter_wd_mm;
    UIValueSetter * UI_setter_wd_ss;

    /* Settings - master */
    UIMenuList * UI_menu_list_master;
    
    UIMenuListItem * UI_settings_master_machine;
    UIMenuListItem * UI_settings_master_sensors;
    UIMenuListItem * UI_settings_master_limits_blowing;
    UIMenuListItem * UI_settings_master_limits_prog;

    /* master, page - 1 */
    UIValueSetter * UI_S_M_type_of_equipment_enum;
    UIValueSetter * UI_S_M_stop_btn_type;       
    UIValueSetter * UI_S_M_is_asyncM_rpm_float;
    UIValueSetter * UI_S_M_plc_language;
    UIValueSetter * UI_S_M_equip_have_wJacket_tempC_sensor;
    UIValueSetter * UI_S_M_reboot_system;
    UIValueSetter * UI_S_M_reset_system;
    UIValueSetter * UI_S_M_hard_reset_system;

    /* master, page - 2 */    
    UIValueSetter * UI_S_M_sensor_batt_min_V;
    UIValueSetter * UI_S_M_sensor_batt_max_V;
    UIValueSetter * UI_S_M_sensor_batt_V_min_12bit;
    UIValueSetter * UI_S_M_sensor_batt_V_max_12bit;
    UIValueSetter * UI_S_M_sensor_tempC_limit_4ma_12bit;
    UIValueSetter * UI_S_M_sensor_tempC_limit_20ma_12bit;
    UIValueSetter * UI_S_M_sensor_tempC_limit_4ma_degrees_C;       
    UIValueSetter * UI_S_M_sensor_tempC_limit_20ma_degrees_C;      
    UIValueSetter * UI_S_M_sensor_dac_rpm_limit_min_12bit;        
    UIValueSetter * UI_S_M_sensor_dac_rpm_limit_max_12bit;        
    UIValueSetter * UI_S_M_sensor_dac_asyncM_rpm_min;              
    UIValueSetter * UI_S_M_sensor_dac_asyncM_rpm_max;

    /* master, page - 3 */              
    UIValueSetter * UI_S_M_blowing_await_ss;                       
    UIValueSetter * UI_S_M_blowing_pump_power_lm;                    
    UIValueSetter * UI_S_M_blowing_limit_ml_max;                  
    UIValueSetter * UI_S_M_blowing_limit_ml_min;                  
    UIValueSetter * UI_S_M_blowing_limit_ss_max;                  
    UIValueSetter * UI_S_M_blowing_limit_ss_min;

    /* master, page - 4 */                  
    UIValueSetter * UI_S_M_wJacket_tempC_limit_max;                
    UIValueSetter * UI_S_M_prog_wJacket_drain_max_ss;              
    UIValueSetter * UI_S_M_prog_on_pause_max_await_ss;            
    UIValueSetter * UI_S_M_prog_await_spite_of_already_runned_ss; 
    UIValueSetter * UI_S_M_prog_limit_heat_tempC_max;              
    UIValueSetter * UI_S_M_prog_limit_heat_tempC_min;              
    UIValueSetter * UI_S_M_prog_limit_chill_tempC_max;             
    UIValueSetter * UI_S_M_prog_limit_chill_tempC_min;             
    UIValueSetter * UI_S_M_prog_any_step_max_durat_ss;            
    UIValueSetter * UI_S_M_prog_coolign_water_safe_mode;            
    UIValueSetter * UI_S_M_prog_heaters_toggle_delay_ss;           
    UIValueSetter * UI_S_M_prog_wJacket_toggle_delay_ss;           

    int16_t get_menu_index() {
        return UI_menu_list_user->get_focused_index();
    }

    uint8_t get_template_tmpe_index()
    {
        int16_t index = get_menu_index() - menu_tmpe_general_start_at;
        return index < 0 ? 0 : (index >= TEMPLATES_COUNT_TMPE ? TEMPLATES_COUNT_TMPE - 1 : index);
    }

    uint8_t get_template_chm_index()
    {
        int16_t index = get_menu_index() - menu_chm_general_start_at;
        return index < 0 ? 0 : (index >= TEMPLATES_COUNT_CHM ? TEMPLATES_COUNT_CHM - 1 : index);
    }

    uint8_t get_template_wd_index()
    {
        int16_t index = get_menu_index() - menu_wd_general_start_at;
        return index < 0 ? 0 : (index >= TEMPLATES_COUNT_WD ? TEMPLATES_COUNT_WD - 1 : index);
    }

public:
    UIService();
    bool get_is_pasteurizer();

private:
    void init_screens();
    void init_blowing_controls();
    void init_settings_user_controls();
    void init_settings_part_datetime();
    void init_settings_part_pump_calibration();
    void init_settings_part_tmpe_templates();
    void init_settings_part_tmpe_wd();
    void init_settings_part_chm_templates();
    void init_settings_master_controls();
    void init_prog_selector_part_tmpe();
    void init_prog_selector_part_chm();
    void display_rt_in_setters();
    void display_wd_in_setters();
};

#endif