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

#define LGFX_USE_V1
#define SCREEN_WIDTH            480U
#define SCREEN_HEIGHT           320U
#define INIT_BUFFER_IN_PSRAM    1

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

class UIService
{
public:
    UIMachineStateBar * UI_machine_state_bar;
    UIDateTime * UI_date_time;
    UINotifyBar * UI_notify_bar;
    UINotificationBar * UI_notification_bar;

    UITaskRoadmapList * UI_task_roadmap_control;
    vector<UITaskListItem> * UI_task_steps;

    UIMenuList * UI_menu_list_user;
    UIMenuListItem * UI_settings_user_datetime;
    UIMenuListItem * UI_settings_user_pump;
    UIMenuListItem * UI_settings_user_pasteurizer_template_1;
    UIValueSetter * UI_Set1;
    UIValueSetter * UI_Set2;
    UIValueSetter * UI_Set3;

    UIBlowingControl * UI_blowing_control;
    vector<UIBlowValListItem*> Blow_vars;

    lv_obj_t* screen;
    
    UIService();

    /* DEMO BEGIN */
    uint8_t * demo_setter_value = new uint8_t;
    
    vector<BlowgunValue> b_vars = {
    BlowgunValue(5000),
    BlowgunValue(5000),
    BlowgunValue(5000),
    BlowgunValue(true, 360)
    };

    vector<ProgramStep> my_demo_task_steps =
    {
        ProgramStep(ProgramStepAimEnum::PASTEUR, 5, 10, 10, true, false),
        ProgramStep(ProgramStepAimEnum::CHILLING, 7, 15, 20, true, false),
        ProgramStep(ProgramStepAimEnum::CUTTING, 9, 20, 30, true, false),
        ProgramStep(ProgramStepAimEnum::MIXING, 11, 25, 40, true, false),
        ProgramStep(ProgramStepAimEnum::HEATING, 13, 30, 50, true, false),
        ProgramStep(ProgramStepAimEnum::DRYING, 15, 35, 60, true, false),
        ProgramStep(ProgramStepAimEnum::PASTEUR, 17, 40, 90, true, false),
        ProgramStep(ProgramStepAimEnum::CHILLING, 19, 45, 120, true, false),
        ProgramStep(ProgramStepAimEnum::CUTTING, 21, 50, 120, true, false)
    };
    /* DEMO END */

private:
    void init_screens();
    void init_blowing_controls();
    void init_settings_user_controls();
};

#endif