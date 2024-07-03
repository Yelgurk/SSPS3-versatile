#ifndef UIBlowingControl_hpp
#define UIBlowingControl_hpp

#include "../UIElement.hpp"

enum class BlowingType : uint8_t { LITER, TIMER };

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

        lv_obj_t * support_icon_up = lv_image_create(support_container);
        lv_obj_align(support_icon_up, LV_ALIGN_TOP_MID, -12, -8);
        lv_obj_set_width(support_icon_up, LV_SIZE_CONTENT);
        lv_obj_set_height(support_icon_up, LV_SIZE_CONTENT);
        lv_image_set_src(support_icon_up, &img_arrow_up);
        lv_image_set_scale(support_icon_up, 128);

        lv_obj_t * blow_val_list_container = lv_obj_create(lv_screen);
        accept_cascade(blow_val_list_container, 158);

        lv_obj_t * blow_selector_header =  lv_label_create(blow_val_list_container);

        lv_obj_t * blow_val_list = lv_label_create(blow_val_list_container);

        lv_obj_t * blow_state_presenter = lv_obj_create(lv_screen);
        accept_cascade(blow_state_presenter, 314);
        lv_obj_set_style_bg_color(blow_state_presenter, COLOR_DUST_BLUE, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t * blow_state_indicator = lv_obj_create(blow_state_presenter);
        lv_obj_remove_flag(blow_state_indicator, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_border_width(blow_state_indicator, 0, 0);
        lv_obj_set_style_radius(blow_state_indicator, 0, 0);
        lv_obj_set_style_bg_color(blow_state_indicator, COLOR_BLUE, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_align(blow_state_indicator, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_obj_set_width(blow_state_indicator, 145);

        lv_obj_t * blow_process_label = lv_label_create(blow_state_presenter);
        lv_obj_set_style_text_font(blow_process_label, &OpenSans_bold_24px, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_align(blow_process_label, LV_ALIGN_CENTER, 0, 0);

        remember_child_element("[blow_proc_bar]", blow_state_indicator);
        remember_child_element("[blow_proc_var]", blow_process_label);
    }

    void set_blow_value(double max_val, double curr_value, BlowingType type, bool reversed_indicator = true)
    {
        static uint32_t indicator_value = 0;
        static char buffer[50];

        if (type == BlowingType::LITER)
            sprintf(buffer, "%.3f л.", curr_value / 1000.0);
        else
        if (type == BlowingType::TIMER)
            sprintf(buffer, "%02d:%02d", (uint32_t)curr_value / 60, (uint32_t)curr_value % 60);

        if (reversed_indicator)
            indicator_value = static_cast<uint32_t>(240.0 / max_val * curr_value);
        else
            indicator_value = static_cast<uint32_t>(240.0 - (240.0 / max_val * curr_value));

        lv_obj_set_height(get_container_content("[blow_proc_bar]"), indicator_value);
        lv_label_set_text(get_container_content("[blow_proc_var]"), buffer);
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