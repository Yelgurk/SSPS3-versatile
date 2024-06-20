#ifndef UIClock_hpp
#define UIClock_hpp

#include "../UIElement.hpp"

class UIClock : public UIElement
{
public:
    UIClock(
        vector<UIAccess> ui_access,
        vector<KeyModel> key_press_actions,
        bool is_focusable,
        lv_obj_t * lv_obj_parent,
        UIElement * ui_parent = nullptr)
        : UIElement{ui_access, key_press_actions, is_focusable, lv_obj_parent, ui_parent}
    {
        lv_obj_set_width(control, 100);
        lv_obj_set_height(control, 40);
        
        lv_obj_set_style_bg_color(control, lv_color_hex(0xFF9966), LV_STATE_FOCUSED);
        
        lv_obj_t * time = lv_label_create(control);
        lv_obj_align(time, LV_ALIGN_TOP_MID, 0, 0);
        lv_label_set_text(time, "12:00:30");

        lv_obj_t * date = lv_label_create(control);
        lv_obj_align(date, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_label_set_text(date, "19.06.2024");

        child_lv_objects.insert({"[time]", time});
        child_lv_objects.insert({"[date]", date});
    }
};

#endif