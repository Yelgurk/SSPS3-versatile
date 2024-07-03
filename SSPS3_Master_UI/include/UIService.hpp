#pragma once

#ifndef UIService_hpp
#define UIService_hpp

#include <Arduino.h>
#include <lvgl.h>
#include "../config/LGFX_SSPS3_V1.hpp"
#include "./UIElement.hpp"

#define INIT_BUFFER_IN_PSRAM    0

#define LGFX_USE_V1
#define SCREEN_WIDTH            480U
#define SCREEN_HEIGHT           320U

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