#ifndef UIClock_hpp
#define UIClock_hpp

#include "../UIElement.hpp"

class UIClock : public UIElement
{
public:
    UIClock(
        vector<EquipmentType> relates_to,
        vector<KeyModel> key_press_actions,
        bool _is_focusable,
        bool _is_container,
        PlaceControlIn bind_to,
        lv_obj_t * lv_screen,
        UIElement * parent_navi = nullptr)
        : UIElement{relates_to, key_press_actions, _is_focusable, _is_container, bind_to, lv_screen, parent_navi}
    {
        lv_obj_set_width(context, 100);
        lv_obj_set_height(context, 40);
        
        lv_obj_set_style_bg_color(context, lv_color_hex(0xFF9966), LV_STATE_FOCUSED);
        
        lv_obj_t * time = lv_label_create(context);
        lv_obj_align(time, LV_ALIGN_TOP_MID, 0, 0);
        lv_label_set_text(time, "12:00:30");

        lv_obj_t * date = lv_label_create(context);
        lv_obj_align(date, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_label_set_text(date, "19.06.2024");

        add_context_child("[time]", time);
        add_context_child("[date]", date);
    }
};

#endif