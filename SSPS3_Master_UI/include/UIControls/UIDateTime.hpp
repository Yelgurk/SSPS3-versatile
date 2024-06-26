#ifndef UIDateTime_hpp
#define UIDateTime_hpp

#include "../UIElement.hpp"

class UIDateTime : public UIElement
{
public:
    UIDateTime(
        vector<EquipmentType> relates_to,
        vector<KeyModel> key_press_actions,
        bool is_focusable,
        lv_obj_t * lv_screen,
        vector<StyleActivator> styles_activator,
        uint32_t width,
        uint32_t height,
        lv_align_t align,
        int32_t offset_x = 0,
        int32_t offset_y = 0
    ) : UIElement {
        relates_to,
        key_press_actions,
        false,
        false,
        true,
        PlaceControlIn::Screen,
        lv_screen,
        nullptr,
        styles_activator
    }
    {
        lv_obj_set_width(get_container(), width);
        lv_obj_set_height(get_container(), height);
        lv_obj_align(get_container(), align, offset_x, offset_y);
        
        lv_obj_t * time = lv_label_create(get_container());
        lv_obj_align(time, LV_ALIGN_TOP_MID, 0, 0);
        lv_label_set_text(time, "12:00:30");

        lv_obj_t * date = lv_label_create(get_container());
        lv_obj_align(date, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_label_set_text(date, "19.06.2024");

        remember_child_element("[time]", time);
        remember_child_element("[date]", date);
    }
};

#endif