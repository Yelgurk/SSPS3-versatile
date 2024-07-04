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
        
        /* USER SUPPORT CONTAINER */
        lv_obj_t * support_container = lv_obj_create(lv_screen);
        accept_cascade(support_container, 145, 0);
        create_support_icons(support_container, &img_arrow_up, -52, 2);
        create_support_icons(support_container, &img_arrow_down, -38, -2);
        create_support_icons(support_container, &img_sort_left, -52, 50);
        create_support_icons(support_container, &img_sort_rigth, -38, 50);
        create_support_icons(support_container, &img_arrow_back, -45, 110, 176);
        create_support_icons(support_container, &img_flag, -45, 175, 176);
        
        lv_obj_t * info_label_1 = create_support_label(support_container, "Выбор", 12);
        lv_obj_t * info_label_2 = create_support_label(support_container, "- / +", 62);
        lv_obj_t * info_label_3 = create_support_label(support_container, "Отмена или\nНазад", 107);
        lv_obj_t * info_label_4 = create_support_label(support_container, "Старт\nраздачи", 172);

        /* BLOWING VAL SELECTOR CONTAINER */
        lv_obj_t * blow_val_list_container = lv_obj_create(lv_screen);
        accept_cascade(blow_val_list_container, 210, 155);

        lv_obj_t * blow_selector_header =  lv_label_create(blow_val_list_container);
        lv_obj_set_style_text_font(blow_selector_header, &OpenSans_bold_20px, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_align(blow_selector_header, LV_ALIGN_TOP_MID, 0, 8);
        lv_label_set_text(blow_selector_header, "Шаблоны раздачи");

        lv_obj_t * blow_val_list = lv_list_create(blow_val_list_container);
        lv_obj_set_width(blow_val_list, 210);
        lv_obj_set_height(blow_val_list, 200);
        lv_obj_align(blow_val_list, LV_ALIGN_TOP_MID, 0, 40);
        lv_obj_set_style_radius(blow_val_list, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(blow_val_list, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_clip_corner(blow_val_list, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(blow_val_list, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_all(blow_val_list, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t * blow_val_list_splitter = lv_obj_create(blow_val_list_container);
        lv_obj_set_style_border_width(blow_val_list_splitter, 0, 0);
        lv_obj_set_style_radius(blow_val_list_splitter, 0, 0);
        lv_obj_set_width(blow_val_list_splitter, 170);
        lv_obj_set_height(blow_val_list_splitter, 1);
        lv_obj_set_style_bg_color(blow_val_list_splitter, COLOR_DARK_GREY, 0);
        lv_obj_align(blow_val_list_splitter, LV_ALIGN_TOP_MID, 0, 40);

        /* BLOWING STATE "BAR" CONTAINER */
        lv_obj_t * blow_state_presenter = lv_obj_create(lv_screen);
        accept_cascade(blow_state_presenter, 85, 375);
        lv_obj_set_style_bg_color(blow_state_presenter, COLOR_DUST_BLUE, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t * blow_state_indicator = lv_obj_create(blow_state_presenter);
        lv_obj_remove_flag(blow_state_indicator, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_border_width(blow_state_indicator, 0, 0);
        lv_obj_set_style_radius(blow_state_indicator, 0, 0);
        lv_obj_set_style_bg_color(blow_state_indicator, COLOR_BLUE, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_align(blow_state_indicator, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_obj_set_width(blow_state_indicator, 85);

        lv_obj_t * blow_process_label = lv_label_create(blow_state_presenter);
        lv_obj_set_style_text_font(blow_process_label, &OpenSans_bold_24px, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_align(blow_process_label, LV_ALIGN_CENTER, 0, 0);

        remember_child_element("[blow_proc_bar]", blow_state_indicator);
        remember_child_element("[blow_proc_var]", blow_process_label);
        remember_child_element("[blow_selector]", blow_val_list);
        set_childs_presenter("[blow_selector]");
    }

    void set_blow_value(float ms_aim, float ms_gone, BlowingType type, float ml_per_ms, bool reversed_indicator = true)
    {
        static uint32_t indicator_value = 0;
        static char buffer[50];

        if (type == BlowingType::LITER)
            sprintf(buffer, "%.3f л.", (ml_per_ms * (ms_aim - ms_gone)) / 1000.f);
        else
        if (type == BlowingType::TIMER)
        {
            int32_t total_ss = ms_aim / 1000.f;
            int32_t gone_ss = ms_gone / 1000.f;
            int32_t left_ss = total_ss - gone_ss;
            sprintf(buffer, "%02d:%02d", left_ss / 60, left_ss % 60);
        }

        if (!reversed_indicator)
            indicator_value = 240.f / ms_aim * ms_gone;
        else
            indicator_value = 240.f - (240.f / ms_aim * ms_gone);

        lv_obj_set_height(get_container_content("[blow_proc_bar]"), indicator_value);
        lv_label_set_text(get_container_content("[blow_proc_var]"), buffer);
    }

private:
    void accept_cascade(lv_obj_t * container, int16_t width, int16_t offset_x)
    {
        lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_pad_all(container, 0, 0);
        lv_obj_set_style_radius(container, 10, 0);
        lv_obj_set_style_bg_color(container, COLOR_WHITE, 0);
        lv_obj_set_style_clip_corner(container, true, 0);
        lv_obj_set_height(container, 240);
        lv_obj_set_width(container, width);
        lv_obj_align(container, LV_ALIGN_LEFT_MID, offset_x + 10, 30);

        lv_obj_set_style_border_width(container, 0, 0);
        lv_obj_set_style_shadow_color(container, lv_color_hex(0x9B9B9B), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_opa(container, 100, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_width(container, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_spread(container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_offset_x(container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_offset_y(container, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    void create_support_icons(lv_obj_t * parent, const void* image_src, int16_t offset_x, int16_t offset_y, uint8_t scale = 160)
    {
        lv_obj_t * support_icon_up = lv_image_create(parent);
        lv_obj_align(support_icon_up, LV_ALIGN_TOP_MID, offset_x, offset_y);
        lv_obj_set_width(support_icon_up, LV_SIZE_CONTENT);
        lv_obj_set_height(support_icon_up, LV_SIZE_CONTENT);
        lv_image_set_src(support_icon_up, image_src);
        lv_image_set_scale(support_icon_up, scale);
    }

    lv_obj_t * create_support_label(lv_obj_t * parent, string text, int16_t offset_y)
    {
        lv_obj_t * label = lv_label_create(parent);
        lv_obj_set_style_text_font(label, &OpenSans_bold_20px, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 50, offset_y);
        lv_label_set_text(label, text.c_str());
        return label;
    }
};

#endif