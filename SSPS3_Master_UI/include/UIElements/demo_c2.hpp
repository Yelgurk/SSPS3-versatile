#ifndef UIClock3_hpp
#define UIClock3_hpp

#include "../UIElement.hpp"

class UIClock3 : public UIElement
{
public:
    UIClock3(
        vector<EquipmentType> relates_to,
        vector<KeyModel> key_press_actions,
        bool _is_focusable,
        bool _is_container,
        PlaceControlIn bind_to,
        lv_obj_t * lv_screen,
        UIElement * parent_navi = nullptr)
        : UIElement{relates_to, key_press_actions, _is_focusable, _is_container, bind_to, lv_screen, parent_navi}
    {
        lv_obj_set_width(context, 250);
        lv_obj_set_height(context, 60);
        
        lv_obj_set_style_bg_color(context, lv_color_hex(0xFF9966), LV_STATE_FOCUSED);
    }
};

#endif