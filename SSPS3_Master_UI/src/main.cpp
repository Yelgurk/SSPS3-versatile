#include "../include/main.hpp"

void setup()
{
    Serial.begin(115200);
    UI_service.init();

    item = lv_obj_create(UI_service.screen);
    lv_obj_set_height(item, 40);
    lv_obj_set_width(item, 40);
    lv_obj_set_align(item, LV_ALIGN_CENTER);

    lv_obj_set_style_bg_color(item, lv_color_hex(0xE52B50), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(item, lv_color_hex(0xFFBF00), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_opa(item, 120, LV_PART_MAIN | LV_STATE_USER_1);
    lv_obj_set_style_opa(item, 0, LV_PART_MAIN | LV_STATE_USER_2);
}

void loop()
{
    lv_task_handler();

    uint8_t x = millis() / 1000 % 4;

    switch (x)
    {
    case 0: LV_OBJ_SHOW(item); break;
    case 1: LV_OBJ_FOCUS(item); break;
    case 2: LV_OBJ_TRANSP(item); break;
    case 3: LV_OBJ_HIDE(item); break;

    default: break;
    }
}