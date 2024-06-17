#include <Arduino.h>
#include <lvgl.h>
#include "../config/LGFX_SSPS3_V4.hpp"

#define LGFX_USE_V1
#define SCREEN_WIDTH            480U
#define SCREEN_HEIGHT           320U
#define SCREEN_BUFFER           (SCREEN_WIDTH * SCREEN_HEIGHT * LV_COLOR_DEPTH) / 32 / 2
#define LV_OBJ_SHOW(lv_obj)     lv_obj_show(lv_obj)  
#define LV_OBJ_FOCUS(lv_obj)    lv_obj_focus(lv_obj)
#define LV_OBJ_TRANSP(lv_obj)   lv_obj_transparent(lv_obj)
#define LV_OBJ_HIDE(lv_obj)     lv_obj_hide(lv_obj)         

static LGFX lcd;
static lv_obj_t* screen;
static uint8_t lv_buff_1[SCREEN_BUFFER];
static uint8_t lv_buff_2[SCREEN_BUFFER];

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

static void lv_obj_remove_states(lv_obj_t* lv_obj)
{
    lv_obj_set_state(lv_obj, LV_STATE_DEFAULT, false);
    lv_obj_set_state(lv_obj, LV_STATE_FOCUSED, false);
    lv_obj_set_state(lv_obj, LV_STATE_USER_1, false);
    lv_obj_set_state(lv_obj, LV_STATE_USER_2, false);
}

static void lv_obj_show(lv_obj_t* lv_obj)
{
    lv_obj_remove_states(lv_obj);
    lv_obj_set_state(lv_obj, LV_STATE_DEFAULT, true);
}

static void lv_obj_focus(lv_obj_t* lv_obj)
{
    lv_obj_remove_states(lv_obj);
    lv_obj_set_state(lv_obj, LV_STATE_FOCUSED, true);
}

static void lv_obj_transparent(lv_obj_t* lv_obj)
{
    lv_obj_remove_states(lv_obj);
    lv_obj_set_state(lv_obj, LV_STATE_USER_1, true);
}

static void lv_obj_hide(lv_obj_t* lv_obj)
{
    lv_obj_remove_states(lv_obj);
    lv_obj_set_state(lv_obj, LV_STATE_USER_2, true);
} 