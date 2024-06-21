#ifndef UIClock5_hpp
#define UIClock5_hpp

#include "../UIElement.hpp"

class UIClock5 : public UIElement
{
public:
    UIClock5(
        vector<EquipmentType> relates_to,
        vector<KeyModel> key_press_actions,
        bool _is_focusable,
        bool _is_container,
        PlaceControlIn bind_to,
        lv_obj_t * lv_screen,
        UIElement * parent_navi = nullptr)
        : UIElement{relates_to, key_press_actions, _is_focusable, _is_container, bind_to, lv_screen, parent_navi}
    {
        lv_obj_align(get_container(), LV_ALIGN_TOP_RIGHT, 0, 0);
        lv_obj_set_width(get_container(), 200);
        lv_obj_set_height(get_container(), 20);
        lv_obj_set_style_bg_color(get_container(), lv_color_hex(0x2200ff), 0);
    }
};

#endif