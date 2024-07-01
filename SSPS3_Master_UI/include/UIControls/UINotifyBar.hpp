#ifndef UINotifyBar_hpp
#define UINotifyBar_hpp

#include "../UIElement.hpp"

class UINotifyBar : public UIElement
{
public:
    UINotifyBar(
        lv_obj_t * lv_screen,
        uint32_t width = 40,
        uint32_t height = 56,
        lv_align_t align = LV_ALIGN_TOP_RIGHT,
        int32_t offset_x = -10,
        int32_t offset_y = 7
    ) : UIElement {
        { EquipmentType::All },
        { },
        false,
        false,
        true,
        PlaceControlIn::Screen,
        lv_screen,
        nullptr,
        { StyleActivator::Shadow }
    }
    {
        lv_obj_set_width(get_container(), width);
        lv_obj_set_height(get_container(), height);
        lv_obj_align(get_container(), align, offset_x, offset_y);
        lv_obj_remove_flag(get_container(), LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_clip_corner(get_container(), true, 0);
        lv_obj_set_style_pad_all(get_container(), -8, 0);

        lv_obj_t * lv_context_text = lv_label_create(get_container());
        lv_obj_set_style_text_font(lv_context_text, &OpenSans_regular_20px, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text(lv_context_text, "3");
        lv_obj_align(lv_context_text, LV_ALIGN_CENTER, 0, 15);
        lv_obj_set_style_opa(lv_context_text, 0, 0);
        lv_obj_set_style_opa(lv_context_text, 255, LV_PART_MAIN | LV_STATE_USER_1);
        lv_obj_set_style_opa(lv_context_text, 255, LV_PART_MAIN | LV_STATE_USER_2);
        lv_obj_set_style_opa(lv_context_text, 255, LV_PART_MAIN | LV_STATE_USER_3);

        lv_obj_t * lv_setter_icon = lv_image_create(get_container());
        lv_image_set_src(lv_setter_icon, &img_checkmark);
        lv_obj_set_style_pad_all(lv_setter_icon, 0, 0);
        lv_obj_set_width(lv_setter_icon, LV_SIZE_CONTENT);
        lv_obj_set_height(lv_setter_icon, LV_SIZE_CONTENT);
        lv_obj_add_flag(lv_setter_icon, LV_OBJ_FLAG_ADV_HITTEST);
        lv_obj_remove_flag(lv_setter_icon, LV_OBJ_FLAG_SCROLLABLE);
        lv_image_set_scale(lv_setter_icon, 160);
        lv_obj_set_style_align(lv_setter_icon, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_align(lv_setter_icon, LV_ALIGN_TOP_MID, LV_PART_MAIN | LV_STATE_USER_1);
        lv_obj_set_style_align(lv_setter_icon, LV_ALIGN_TOP_MID, LV_PART_MAIN | LV_STATE_USER_2);
        lv_obj_set_style_align(lv_setter_icon, LV_ALIGN_TOP_MID, LV_PART_MAIN | LV_STATE_USER_3);

        lv_obj_set_style_bg_color(get_container(), COLOR_WHITE, 0);
        lv_obj_set_style_bg_color(get_container(), COLOR_SKY_BLUE, LV_PART_MAIN | LV_STATE_USER_1);
        lv_obj_set_style_bg_color(get_container(), COLOR_YELLOW_SMOKE, LV_PART_MAIN | LV_STATE_USER_2);
        lv_obj_set_style_bg_color(get_container(), COLOR_PINK, LV_PART_MAIN | LV_STATE_USER_3);
        lv_obj_set_style_opa(get_container(), 255, 0);
        lv_obj_set_style_opa(get_container(), 255, LV_STATE_USER_1);
        lv_obj_set_style_opa(get_container(), 255, LV_STATE_USER_2);
        lv_obj_set_style_opa(get_container(), 255, LV_STATE_USER_3);

        remember_child_element("[context_count]", lv_context_text);
        remember_child_element("[context_icon]", lv_setter_icon);

        //control_set_values_notify_bar(LV_STATE_DEFAULT, 0);
        //control_set_values_notify_bar(LV_STATE_USER_1, 1);
        control_set_values_notify_bar(LV_STATE_USER_2, 2);
        //control_set_values_notify_bar(LV_STATE_USER_3, 3);
    }

    void control_set_values_notify_bar(_lv_state_t selector, uint8_t count)
    {
        set_bar_style(selector);
        lv_label_set_text(get_container_content("[context_count]"), to_string(count).c_str());
    }

private:
    void clear_all_flags(lv_obj_t * label)
    {
        lv_obj_set_state(label, LV_STATE_USER_1, false);
        lv_obj_set_state(label, LV_STATE_USER_2, false);
        lv_obj_set_state(label, LV_STATE_USER_3, false);
    }

    void set_bar_style(_lv_state_t selector)
    {
        clear_all_flags(get_container_content("[context_count]"));
        clear_all_flags(get_container_content("[context_icon]"));
        clear_all_flags(get_container());
        if (selector != LV_STATE_DEFAULT)
        {
            lv_obj_set_state(get_container_content("[context_count]"), selector, true);
            lv_obj_set_state(get_container_content("[context_icon]"), selector, true);
            lv_obj_set_state(get_container(), selector, true);
        }

        switch (selector)
        {
        case LV_STATE_USER_1:   lv_image_set_src(get_container_content("[context_icon]"), &img_info); break;
        case LV_STATE_USER_2:   lv_image_set_src(get_container_content("[context_icon]"), &img_warning); break;
        case LV_STATE_USER_3:   lv_image_set_src(get_container_content("[context_icon]"), &img_error); break;
        default:                lv_image_set_src(get_container_content("[context_icon]"), &img_checkmark); break;
        }
    }
};

#endif