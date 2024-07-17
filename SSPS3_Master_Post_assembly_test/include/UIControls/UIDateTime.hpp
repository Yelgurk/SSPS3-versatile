#ifndef UIDateTime_hpp
#define UIDateTime_hpp

#include "../UIElement.hpp"

class UIDateTime : public UIElement
{
public:
    UIDateTime(
        lv_obj_t * lv_screen,
        uint32_t width = 90,
        uint32_t height = 56,
        lv_align_t align = LV_ALIGN_TOP_RIGHT,
        int32_t offset_x = -55,
        int32_t offset_y = 7
    ) : UIElement {
        { EquipmentType::All },
        { },
        false,
        false,
        true,
        PlaceControlIn::Screen,
        lv_screen,
        nullptr,
        { StyleActivator::Shadow }
    }
    {
        lv_obj_set_width(get_container(), width);
        lv_obj_set_height(get_container(), height);
        lv_obj_align(get_container(), align, offset_x, offset_y);
        
        lv_obj_t * time = lv_label_create(get_container());
        lv_obj_set_style_text_font(time, &OpenSans_regular_24px, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_align(time, LV_ALIGN_TOP_MID, 0, 3);
        lv_label_set_text(time, "xx:xx:xx");

        lv_obj_t * date = lv_label_create(get_container());
        lv_obj_set_style_text_font(date, &OpenSans_regular_16px, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_align(date, LV_ALIGN_BOTTOM_MID, 0, -3);
        lv_label_set_text(date, "xx.xx.xxxx");

        remember_child_element("[time]", time);
        remember_child_element("[date]", date);

        control_set_values_date_time(8, 5, 35, 1, 7, 2024);
    }

    void control_set_values_date_time(uint8_t hh, uint8_t mm, uint8_t ss, uint8_t dd, uint8_t MM, uint16_t yyyy)
    {
        static char buffer[20];

        sprintf(buffer, "%02d:%02d:%02d", hh, mm, ss);
        lv_label_set_text(get_container_content("[time]"), buffer);

        sprintf(buffer, "%02d / %02d / %04d", dd, MM, yyyy);
        lv_label_set_text(get_container_content("[date]"), buffer);
    }
};

#endif