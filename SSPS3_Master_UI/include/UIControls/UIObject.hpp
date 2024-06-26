#ifndef UIObject_hpp
#define UIObject_hpp

#include "../UIElement.hpp"

class UIObject : public UIElement
{
public:
    UIObject(
        vector<EquipmentType> relates_to,
        vector<KeyModel> key_press_actions,
        bool is_focusable,
        bool is_selectable,
        bool is_container,
        PlaceControlIn bind_to,
        lv_obj_t * lv_screen,
        UIElement * parent_navi = nullptr,
        vector<StyleActivator> styles_activator,
        uint32_t width,
        uint32_t height,
        lv_align_t align,
        int32_t offset_x = 0,
        int32_t offset_y = 0
    ) : UIElement {
        relates_to,
        key_press_actions,
        is_focusable,
        is_selectable,
        is_container,
        bind_to,
        lv_screen,
        parent_navi,
        styles_activator
    }
    {
        lv_obj_set_width(get_container(), width);
        lv_obj_set_height(get_container(), height);
        lv_obj_align(get_container(), align, offset_x, offset_y);
        lv_obj_set_style_bg_color(get_container(), COLOR_WHITE_SMOKE, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
};

#endif