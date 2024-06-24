#ifndef UIClockx_hpp
#define UIClockx_hpp

#include "../UIElement.hpp"

class UIClockx : public UIElement
{
public:
    UIClockx(
        vector<EquipmentType> relates_to,
        vector<KeyModel> key_press_actions,
        PlaceControlIn bind_to,
        lv_obj_t * lv_screen,
        UIElement * parent_navi = nullptr)
        : UIElement{relates_to, key_press_actions, false, false, true, bind_to, lv_screen, parent_navi}
    {
        lv_obj_set_width(get_container(), 480);
        lv_obj_set_height(get_container(), 320);
        lv_obj_align(get_container(), LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_bg_opa(get_container(), 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    UIClockx * set_wh(uint16_t width, uint16_t height)
    {
        lv_obj_set_width(get_container(), width);
        lv_obj_set_height(get_container(), height);
        return this;
    }

    UIClockx * set_align(_lv_align_t align_to, int32_t offset_x = 0, int32_t offset_y = 0)
    {
        lv_obj_align(get_container(), align_to, offset_x, offset_y);
        return this;
    }
};

#endif