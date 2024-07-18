#pragma once

#ifndef UIService_hpp
#define UIService_hpp

#include "../config/LGFX_SSPS3_V1.hpp"
#include <lvgl.h>
#include <Arduino.h>

#include "UIElement.hpp"
#include "UIControls/UIBlowingControl.hpp"
#include "UIControls/UIBlowValListItem.hpp"
#include "UIControls/UIDateTime.hpp"
#include "UIControls/UIFlowGunProgressBar.hpp"
#include "UIControls/UIMachineStateBar.hpp"
#include "UIControls/UIMenuList.hpp"
#include "UIControls/UIMenuListItem.hpp"
#include "UIControls/UINotificationBar.hpp"
#include "UIControls/UINotifyBar.hpp"
#include "UIControls/UIObject.hpp"
#include "UIControls/UIScreen.hpp"
#include "UIControls/UITaskListItem.hpp"
#include "UIControls/UITaskRoadmapList.hpp"
#include "UIControls/UIValueSetter.hpp"

#include "BlowingControl.hpp"
#include "ProgramControl.hpp"
#include "FRAM_DB.hpp"
#include "DS3231.h"

#define LGFX_USE_V1
#define SCREEN_WIDTH            480U
#define SCREEN_HEIGHT           320U
#define INIT_BUFFER_IN_PSRAM    1
#define TEMPLATES_COUNT_TMPE    6
#define TEMPLATES_COUNT_WD      3
#define TEMPLATES_COUNT_CHM     10

#if INIT_BUFFER_IN_PSRAM == 0
    #define SCREEN_BUFFER           (SCREEN_WIDTH * SCREEN_HEIGHT * LV_COLOR_DEPTH) / 24 / 2    
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

extern ProgramControl * Program_control;
extern BlowingControl * Blowing_control;
extern DS3231 * rtc;
extern S_DateTime * dt_rt;

class UIService
{
public:
    lv_obj_t* screen;

    UIMachineStateBar * UI_machine_state_bar;
    UIDateTime * UI_date_time;
    UINotifyBar * UI_notify_bar;
    UINotificationBar * UI_notification_bar;
    UITaskRoadmapList * UI_task_roadmap_control;
    UIBlowingControl * UI_blowing_control;

    vector<UITaskListItem> * UI_task_steps;
    vector<UIBlowValListItem*> Blow_vars;

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

    /* DEMO BEGIN */
    /*
    vector<BlowgunValue> b_vars = {
    BlowgunValue(5000),
    BlowgunValue(5000),
    BlowgunValue(5000),
    BlowgunValue(true, 360)
    };

    vector<ProgramStep> my_demo_task_steps =
    {
        ProgramStep(ProgramStepAimEnum::PASTEUR, 5, 10, 10, false, false, true),
        ProgramStep(ProgramStepAimEnum::CHILLING, 7, 15, 20, false, false, true),
        ProgramStep(ProgramStepAimEnum::CUTTING, 9, 20, 30, false, false, true),
        ProgramStep(ProgramStepAimEnum::MIXING, 11, 25, 40, false, false, true),
        ProgramStep(ProgramStepAimEnum::HEATING, 13, 30, 50, false, false, true),
        ProgramStep(ProgramStepAimEnum::DRYING, 15, 35, 60, false, false, true),
        ProgramStep(ProgramStepAimEnum::PASTEUR, 17, 40, 90, false, false, true),
        ProgramStep(ProgramStepAimEnum::CHILLING, 19, 45, 120, false, false, true),
        ProgramStep(ProgramStepAimEnum::CUTTING, 21, 50, 120, false, false, true)
    };
    */
    /* DEMO END */

public:
    UIService();

private:
    void init_screens();
    void init_blowing_controls();
    void init_settings_user_controls();
    void init_settings_part_datetime();
    void init_settings_part_pump_calibration();
    void init_settings_part_tmpe_templates();
    void init_settings_part_tmpe_wd();
    void init_settings_part_chm_templates();
    void display_rt_in_setters();
    void display_wd_in_setters();
};

#endif