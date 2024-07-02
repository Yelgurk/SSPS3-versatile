#ifndef UIBlowingControl_hpp
#define UIBlowingControl_hpp

#include "../UIElement.hpp"

class UIBlowingControl : public UIElement
{
public:
    UIBlowingControl(
        vector<KeyModel> key_press_actions,
        lv_obj_t * lv_screen
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
        lv_obj_set_style_opa(get_container(), 0, 0);
        
        lv_obj_t * support_container = lv_obj_create(lv_screen);
        accept_cascade(support_container, 0);

        lv_obj_t * blow_val_list_container = lv_obj_create(lv_screen);
        accept_cascade(blow_val_list_container, 158);

        lv_obj_t * blow_selector_header =  lv_label_create(blow_val_list_container);

        lv_obj_t * blow_val_list = lv_label_create(blow_val_list_container);

        lv_obj_t * blow_state_presenter = lv_bar_create(lv_screen);
        accept_cascade(blow_state_presenter, 314);
        lv_bar_set_value(blow_state_presenter, 400, LV_ANIM_ON);
        lv_bar_set_range(blow_state_presenter, 0, 1000);
        lv_obj_set_style_radius(blow_state_presenter, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(blow_state_presenter, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(blow_state_presenter, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(blow_state_presenter, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(blow_state_presenter, COLOR_DUST_BLUE, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(blow_state_presenter, COLOR_BLUE, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    }

private:
    void accept_cascade(lv_obj_t * container, int16_t offset_x)
    {
        lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_pad_all(container, 0, 0);
        lv_obj_set_style_radius(container, 10, 0);
        lv_obj_set_style_bg_color(container, COLOR_WHITE, 0);
        lv_obj_set_style_clip_corner(container, true, 0);
        lv_obj_set_height(container, 240);
        lv_obj_set_width(container, 145);
        lv_obj_align(container, LV_ALIGN_LEFT_MID, offset_x + 10, 30);

        lv_obj_set_style_border_width(container, 0, 0);
        lv_obj_set_style_shadow_color(container, lv_color_hex(0x9B9B9B), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_opa(container, 100, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_width(container, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_spread(container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_offset_x(container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_offset_y(container, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
};

#endif