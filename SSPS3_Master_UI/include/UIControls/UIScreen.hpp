#ifndef UIScreen_hpp
#define UIScreen_hpp

#include "../UIElement.hpp"

class UIScreen : public UIElement
{
public:
    UIScreen(
        lv_obj_t * lv_screen,
        EquipmentType relates_to,
        vector<KeyModel> key_press_actions
    ) : UIElement {
        { relates_to },
        key_press_actions,
        false,
        false,
        true,
        PlaceControlIn::Screen,
        lv_screen
    }
    {
        lv_obj_set_width(get_container(), 480);
        lv_obj_set_height(get_container(), 320);
        lv_obj_align(get_container(), LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_bg_color(get_container(), COLOR_WHITE, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
};

#endif