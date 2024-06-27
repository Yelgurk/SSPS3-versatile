#pragma once

#ifndef UIService_hpp
#define UIService_hpp

#include <Arduino.h>
#include <lvgl.h>
#include "../config/LGFX_SSPS3_V1.hpp"
#include "./UIElement.hpp"

#define LGFX_USE_V1
#define SCREEN_WIDTH            480U
#define SCREEN_HEIGHT           320U
#define SCREEN_BUFFER           (SCREEN_WIDTH * SCREEN_HEIGHT * LV_COLOR_DEPTH) / 32 / 2    

static uint8_t lv_buff_1[SCREEN_BUFFER];
static uint8_t lv_buff_2[SCREEN_BUFFER];
static LGFX lcd;

static uint32_t arduino_tick_get_cb(void);

static void lcd_flush_cb(lv_display_t* display, const lv_area_t* area, unsigned char* data)
{
    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);
    lv_draw_sw_rgb565_swap(data, w*h);
    lcd.pushImage(area->x1, area->y1, w, h, (uint16_t*)data);
    lv_display_flush_ready(display);
}

class UIService
{
public:
    lv_obj_t* screen;
    
    void init();
};

#endif