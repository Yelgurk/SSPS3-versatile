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
        lv_obj_align(get_container(), LV_ALIGN_TOP_LEFT, 100, 0);
        lv_obj_set_width(get_container(), 300);
        lv_obj_set_height(get_container(), 240);

        lv_obj_t * listik = lv_list_create(get_container());
        lv_obj_set_style_radius(listik, 0, 0);
        lv_obj_set_style_border_width(listik, 0, 0);
        lv_obj_align(listik, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_set_width(listik, 300);
        lv_obj_set_height(listik, 240);

        this->remember_container_child("[list]", listik);
        this->set_child_presenter("[list]");
    }
};

#endif