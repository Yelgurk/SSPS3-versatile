#ifndef UIClock2_hpp
#define UIClock2_hpp

#include "../UIElement.hpp"

class UIClock2 : public UIElement
{
public:
    UIClock2(
        vector<EquipmentType> relates_to,
        vector<KeyModel> key_press_actions,
        bool _is_focusable,
        bool _is_container,
        PlaceControlIn bind_to,
        lv_obj_t * lv_screen,
        UIElement * parent_navi = nullptr)
        : UIElement{relates_to, key_press_actions, _is_focusable, _is_container, bind_to, lv_screen, parent_navi}
    {
        lv_obj_set_width(get_container(), 250);
        lv_obj_set_height(get_container(), 60);
        lv_obj_set_style_bg_color(get_container(), lv_color_hex(0x770077), 0);
    }
};

#endif