#ifndef UIList_hpp
#define UIList_hpp

#include "../UIElement.hpp"

class UIList : public UIElement
{
public:
    UIList(
        vector<EquipmentType> relates_to,
        vector<KeyModel> key_press_actions,
        UIElement * parent_navi,
        vector<StyleActivator> styles_activator,
        uint32_t width,
        uint32_t height,
        lv_align_t align,
        int32_t offset_x = 0,
        int32_t offset_y = 0
    ) : UIElement {
        relates_to,
        key_press_actions,
        true,
        true,
        false,
        PlaceControlIn::Control,
        parent_navi->get_screen(),
        parent_navi,
        styles_activator
    }
    {
        lv_obj_align(get_container(), LV_ALIGN_TOP_LEFT, 100, 0);
        lv_obj_set_width(get_container(), width);
        lv_obj_set_height(get_container(), height);
        lv_obj_align(get_container(), align, offset_x, offset_y);

        lv_obj_t * list_presenter = lv_list_create(get_container());
        lv_obj_set_style_radius(list_presenter, 0, 0);
        lv_obj_set_style_border_width(list_presenter, 0, 0);
        lv_obj_align(list_presenter, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_set_width(list_presenter, width);
        lv_obj_set_height(list_presenter, height);
        lv_obj_set_style_bg_color(list_presenter, COLOR_WHITE_SMOKE, 0);

        this->remember_child_element("[list_presenter]", list_presenter);
        this->set_childs_presenter("[list_presenter]");
    }
};

#endif