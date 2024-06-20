#ifndef UIClock1_hpp
#define UIClock1_hpp

#include "../UIElement.hpp"

class UIClock1 : public UIElement
{
public:
    UIClock1(
        vector<EquipmentType> relates_to,
        vector<KeyModel> key_press_actions,
        bool _is_focusable,
        bool _is_container,
        PlaceControlIn bind_to,
        lv_obj_t * lv_screen,
        UIElement * parent_navi = nullptr)
        : UIElement{relates_to, key_press_actions, _is_focusable, _is_container, bind_to, lv_screen, parent_navi}
    {
        if (bind_to == PlaceControlIn::Control && parent_navi != nullptr)
            context = lv_list_create(parent_navi->get_container());
        else
            context = lv_list_create(lv_screen);

        lv_obj_align(context, LV_ALIGN_TOP_LEFT, 100, 0);
        lv_obj_set_width(context, 300);
        lv_obj_set_height(context, 240);
    }
};

#endif