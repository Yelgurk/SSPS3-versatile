#ifndef UIProgramSelectorControl_hpp
#define UIProgramSelectorControl_hpp

#include "../UIElement.hpp"

class UIProgramSelectorControl : public UIElement
{
public:
    UIProgramSelectorControl(
        lv_obj_t * lv_screen,
        vector<KeyModel> key_press_actions
    ) : UIElement {
        { EquipmentType::All },
        key_press_actions,
        false,
        false,
        true,
        PlaceControlIn::Screen,
        lv_screen,
        nullptr,
        { StyleActivator::None }
    }
    {
        static uint16_t height = 240;
        static uint16_t width = 460;

        lv_obj_set_width(get_container(), width);
        lv_obj_set_height(get_container(), height);
        lv_obj_align(get_container(), LV_ALIGN_BOTTOM_MID, 0, -10);
        lv_obj_remove_flag(get_container(), LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_opa(get_container(), 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(get_container(), COLOR_WHITE, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_clip_corner(get_container(), true, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(get_container(), 0, LV_PART_MAIN | LV_STATE_DEFAULT);

#ifndef SSPS3_IS_CHEAP_SOLUTION_YES
        lv_obj_set_style_radius(get_container(), 10, LV_PART_MAIN | LV_STATE_DEFAULT);
        
        lv_obj_set_style_shadow_color(get_container(), COLOR_MEDIUM_GREY, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_opa(get_container(), 155, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_width(get_container(), 15, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_spread(get_container(), 1, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_offset_x(get_container(), 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_offset_y(get_container(), 5, LV_PART_MAIN | LV_STATE_DEFAULT);
#endif

        lv_obj_t * lv_list = lv_list_create(get_container());
        lv_obj_set_width(lv_list, width);
        lv_obj_set_height(lv_list, height);
        lv_obj_set_align(lv_list, LV_ALIGN_CENTER);
        lv_obj_set_style_radius(lv_list, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(lv_list, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_clip_corner(lv_list, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(lv_list, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_all(lv_list, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        remember_child_element("[prog_selector_list]", lv_list);
        set_childs_presenter("[prog_selector_list]");
    }
};

#endif