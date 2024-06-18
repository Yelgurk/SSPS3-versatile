#include "../include/UIService.hpp"

static uint32_t arduino_tick_get_cb(void) {
    return millis();
}

void UIService::init()
{
    lcd.init();
    lcd.setRotation(0);

    lv_init();
    lv_tick_set_cb(arduino_tick_get_cb);

    lv_display_t* disp = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(
        disp,
        [](lv_display_t* display, const lv_area_t* area, unsigned char* data) {
            lcd_flush_cb(display, area, data);
        }
    );
    lv_display_set_buffers(disp, lv_buff_1, lv_buff_2, SCREEN_BUFFER, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_disp_set_default(disp);

    screen = lv_obj_create(NULL);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xAAAAAA), LV_PART_MAIN | LV_STATE_DEFAULT);
        
    lv_disp_load_scr(screen);
}