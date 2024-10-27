#ifndef UICheapRoadmap_hpp
#define UICheapRoadmap_hpp

#include "../UIElement.hpp"

enum class CheapTaskStateEnum : uint8_t { AWAIT, RUNNED, PAUSE, DONE, ERROR };

class UICheapRoadmap : public UIElement
{
public:
    UICheapRoadmap(
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
        lv_obj_set_width(get_container(), 460);
        lv_obj_set_height(get_container(), 240);
        lv_obj_set_x(get_container(), 0);
        lv_obj_set_y(get_container(), 30);
        lv_obj_set_align(get_container(), LV_ALIGN_CENTER);
        lv_obj_clear_flag(get_container(), LV_OBJ_FLAG_SCROLLABLE);      /// Flags
        lv_obj_set_style_radius(get_container(), 20, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(get_container(), COLOR_GREY, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(get_container(), 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_clip_corner(get_container(), true, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(get_container(), 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_color(get_container(), COLOR_MEDIUM_GREY, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_opa(get_container(), 155, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_width(get_container(), 15, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_spread(get_container(), 1, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_ofs_x(get_container(), 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_ofs_y(get_container(), 5, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t * lv_col_header_label_name = lv_label_create(get_container());
        lv_label_set_long_mode(lv_col_header_label_name, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_set_flex_grow(lv_col_header_label_name, 1);
        lv_obj_set_width(lv_col_header_label_name, LV_SIZE_CONTENT);   /// 1
        lv_obj_set_style_max_width(lv_col_header_label_name, 210, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_height(lv_col_header_label_name, LV_SIZE_CONTENT);    /// 1
        lv_obj_set_x(lv_col_header_label_name, -105);
        lv_obj_set_y(lv_col_header_label_name, -78);
        lv_label_set_text(lv_col_header_label_name, "Шаблон");
        lv_obj_set_align(lv_col_header_label_name, LV_ALIGN_CENTER);
        lv_obj_set_style_text_font(lv_col_header_label_name, &OpenSans_bold_24px, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t * lv_task_progress_state_percentage = lv_label_create(get_container());
        lv_obj_set_width(lv_task_progress_state_percentage, LV_SIZE_CONTENT);   /// 1
        lv_obj_set_height(lv_task_progress_state_percentage, LV_SIZE_CONTENT);    /// 1
        lv_obj_set_x(lv_task_progress_state_percentage, -50);
        lv_obj_set_y(lv_task_progress_state_percentage, 105);
        lv_obj_set_align(lv_task_progress_state_percentage, LV_ALIGN_CENTER);
        lv_label_set_text(lv_task_progress_state_percentage, "77%");
        lv_obj_set_style_text_font(lv_task_progress_state_percentage, &OpenSans_bold_24px, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t * lv_task_progress_state_duration = lv_label_create(get_container());
        lv_obj_set_width(lv_task_progress_state_duration, LV_SIZE_CONTENT);   /// 1
        lv_obj_set_height(lv_task_progress_state_duration, LV_SIZE_CONTENT);    /// 1
        lv_obj_set_x(lv_task_progress_state_duration, 70);
        lv_obj_set_y(lv_task_progress_state_duration, 105);
        lv_obj_set_align(lv_task_progress_state_duration, LV_ALIGN_CENTER);
        lv_label_set_text(lv_task_progress_state_duration, "24:59:59");
        lv_obj_set_style_text_font(lv_task_progress_state_duration, &OpenSans_bold_24px, LV_PART_MAIN | LV_STATE_DEFAULT);

        remember_child_element("[header_name]", lv_col_header_label_name);
        remember_child_element("[state_percentage]", lv_task_progress_state_percentage);
        remember_child_element("[state_duration]", lv_task_progress_state_duration);
    }

    void set_task_header_name(string name)
    {
        lv_label_set_text(get_container_content("[header_name]"), name.c_str());
    }

    void set_task_state_values(double percentage_done, int64_t task_gone_ss, CheapTaskStateEnum state)
    {
        static char buffer[50];

        sprintf(buffer, "%.2f", percentage_done);
        lv_label_set_text(get_container_content("[state_percentage]"), (string(buffer) + "%").c_str());

        sprintf(buffer, "%02d:%02d:%02d", (uint32_t)task_gone_ss / (60 * 60), (uint32_t)task_gone_ss / 60, (uint32_t)task_gone_ss % 60);
        lv_label_set_text(get_container_content("[state_duration]"), buffer);

        /* here's code for label accordingly to main task state */
    }
};

#endif