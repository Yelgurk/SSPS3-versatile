#ifndef UITaskRoadmapList_hpp
#define UITaskRoadmapList_hpp

#include "../UIElement.hpp"
#include "./UITaskListItem.hpp"

enum class TaskStateEnum : uint8_t { AWAIT, RUNNED, PAUSE, DONE, ERROR };

class UITaskRoadmapList : public UIElement
{
private:
    vector<UITaskListItem*> _collection;

    void clear_list()
    {
        this->navi_back();

        for (uint16_t i = 0; i < _collection.size(); i++)
        {
            _collection.at(i)->delete_ui_element(false);
            delete _collection.at(i);   
        }
        _collection.clear();

        clear_ui_childs();
        lv_obj_clean(this->get_navi_childs_presenter());
    }

public:
    UITaskRoadmapList(
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
    
        lv_obj_t * lv_task_progress_bar_indicator = lv_arc_create(get_container());
        lv_obj_set_width(lv_task_progress_bar_indicator, 460);
        lv_obj_set_height(lv_task_progress_bar_indicator, 460);
        lv_obj_set_align(lv_task_progress_bar_indicator, LV_ALIGN_CENTER);
        lv_arc_set_range(lv_task_progress_bar_indicator, 0, 10000);
        lv_arc_set_bg_angles(lv_task_progress_bar_indicator, 91, 90);
        lv_arc_set_value(lv_task_progress_bar_indicator, 7500);
        lv_obj_set_style_pad_all(lv_task_progress_bar_indicator, -24, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_arc_width(lv_task_progress_bar_indicator, 170, LV_PART_INDICATOR | LV_STATE_DEFAULT);
        lv_obj_set_style_arc_rounded(lv_task_progress_bar_indicator, false, LV_PART_INDICATOR | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(lv_task_progress_bar_indicator, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
        lv_obj_set_style_arc_opa(lv_task_progress_bar_indicator, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_arc_width(lv_task_progress_bar_indicator, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_arc_color(lv_task_progress_bar_indicator, COLOR_WHITE, LV_PART_INDICATOR | LV_STATE_DEFAULT);
        lv_obj_set_style_arc_color(lv_task_progress_bar_indicator, COLOR_BLUE, LV_PART_INDICATOR | LV_STATE_USER_1);
        lv_obj_set_style_arc_color(lv_task_progress_bar_indicator, COLOR_YELLOW, LV_PART_INDICATOR | LV_STATE_USER_2);
        lv_obj_set_style_arc_color(lv_task_progress_bar_indicator, COLOR_GREEN, LV_PART_INDICATOR | LV_STATE_USER_3);
        lv_obj_set_style_arc_color(lv_task_progress_bar_indicator, COLOR_RED, LV_PART_INDICATOR | LV_STATE_USER_4);

        lv_obj_t * lv_task_progress_bar_content_presenter = lv_obj_create(get_container());
        lv_obj_set_width(lv_task_progress_bar_content_presenter, 420);
        lv_obj_set_height(lv_task_progress_bar_content_presenter, 190);
        lv_obj_set_x(lv_task_progress_bar_content_presenter, 0);
        lv_obj_set_y(lv_task_progress_bar_content_presenter, -5);
        lv_obj_set_align(lv_task_progress_bar_content_presenter, LV_ALIGN_CENTER);
        lv_obj_clear_flag(lv_task_progress_bar_content_presenter, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
        lv_obj_set_style_radius(lv_task_progress_bar_content_presenter, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
        
        lv_obj_set_style_border_width(lv_task_progress_bar_content_presenter, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_color(lv_task_progress_bar_content_presenter, COLOR_MEDIUM_GREY, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_opa(lv_task_progress_bar_content_presenter, 155, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_width(lv_task_progress_bar_content_presenter, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_spread(lv_task_progress_bar_content_presenter, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_ofs_x(lv_task_progress_bar_content_presenter, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_ofs_y(lv_task_progress_bar_content_presenter, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_left(lv_task_progress_bar_content_presenter, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_right(lv_task_progress_bar_content_presenter, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_top(lv_task_progress_bar_content_presenter, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_bottom(lv_task_progress_bar_content_presenter, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_clip_corner(lv_task_progress_bar_content_presenter, true, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t * lv_col_splitter_3 = lv_obj_create(lv_task_progress_bar_content_presenter);
        lv_obj_set_width(lv_col_splitter_3, 130);
        lv_obj_set_height(lv_col_splitter_3, 190);
        lv_obj_set_align(lv_col_splitter_3, LV_ALIGN_RIGHT_MID);
        lv_obj_clear_flag(lv_col_splitter_3, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
        lv_obj_set_style_radius(lv_col_splitter_3, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(lv_col_splitter_3, lv_color_hex(0xDCDCC8), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(lv_col_splitter_3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(lv_col_splitter_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_left(lv_col_splitter_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_right(lv_col_splitter_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_top(lv_col_splitter_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_bottom(lv_col_splitter_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t * lv_col_splitter_2 = lv_obj_create(lv_task_progress_bar_content_presenter);
        lv_obj_set_width(lv_col_splitter_2, 50);
        lv_obj_set_height(lv_col_splitter_2, 190);
        lv_obj_set_x(lv_col_splitter_2, -110);
        lv_obj_set_y(lv_col_splitter_2, 0);
        lv_obj_set_align(lv_col_splitter_2, LV_ALIGN_RIGHT_MID);
        lv_obj_clear_flag(lv_col_splitter_2, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
        lv_obj_set_style_radius(lv_col_splitter_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(lv_col_splitter_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_left(lv_col_splitter_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_right(lv_col_splitter_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_top(lv_col_splitter_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_bottom(lv_col_splitter_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t * lv_col_splitter_1 = lv_obj_create(lv_task_progress_bar_content_presenter);
        lv_obj_set_width(lv_col_splitter_1, 50);
        lv_obj_set_height(lv_col_splitter_1, 190);
        lv_obj_set_x(lv_col_splitter_1, -160);
        lv_obj_set_y(lv_col_splitter_1, 0);
        lv_obj_set_align(lv_col_splitter_1, LV_ALIGN_RIGHT_MID);
        lv_obj_clear_flag(lv_col_splitter_1, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
        lv_obj_set_style_radius(lv_col_splitter_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(lv_col_splitter_1, lv_color_hex(0xDCDCC8), LV_PART_MAIN | LV_STATE_DEFAULT); //0xC8C8C8
        lv_obj_set_style_bg_opa(lv_col_splitter_1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(lv_col_splitter_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_left(lv_col_splitter_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_right(lv_col_splitter_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_top(lv_col_splitter_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_bottom(lv_col_splitter_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t * lv_col_splitter_name = lv_obj_create(lv_task_progress_bar_content_presenter);
        lv_obj_set_width(lv_col_splitter_name, 420);
        lv_obj_set_height(lv_col_splitter_name, 2);
        lv_obj_set_x(lv_col_splitter_name, 0);
        lv_obj_set_y(lv_col_splitter_name, 35);
        lv_obj_set_align(lv_col_splitter_name, LV_ALIGN_TOP_MID);
        lv_obj_clear_flag(lv_col_splitter_name, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
        lv_obj_set_style_radius(lv_col_splitter_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(lv_col_splitter_name, lv_color_hex(0x323232), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(lv_col_splitter_name, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(lv_col_splitter_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t * lv_col_header_icon_fan = lv_img_create(lv_col_splitter_1);
        lv_img_set_src(lv_col_header_icon_fan, &img_fan);
        lv_obj_set_width(lv_col_header_icon_fan, LV_SIZE_CONTENT);   /// 36
        lv_obj_set_height(lv_col_header_icon_fan, LV_SIZE_CONTENT);    /// 36
        lv_obj_set_x(lv_col_header_icon_fan, 0);
        lv_obj_set_y(lv_col_header_icon_fan, -6);
        lv_obj_set_align(lv_col_header_icon_fan, LV_ALIGN_TOP_MID);
        lv_obj_add_flag(lv_col_header_icon_fan, LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
        lv_obj_clear_flag(lv_col_header_icon_fan, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
        lv_img_set_zoom(lv_col_header_icon_fan, 144);

        lv_obj_t * lv_col_header_icon_tempC = lv_img_create(lv_col_splitter_2);
        lv_img_set_src(lv_col_header_icon_tempC, &img_tempC);
        lv_obj_set_width(lv_col_header_icon_tempC, LV_SIZE_CONTENT);   /// 48
        lv_obj_set_height(lv_col_header_icon_tempC, LV_SIZE_CONTENT);    /// 48
        lv_obj_set_x(lv_col_header_icon_tempC, 0);
        lv_obj_set_y(lv_col_header_icon_tempC, -6);
        lv_obj_set_align(lv_col_header_icon_tempC, LV_ALIGN_TOP_MID);
        lv_obj_add_flag(lv_col_header_icon_tempC, LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
        lv_obj_clear_flag(lv_col_header_icon_tempC, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
        lv_img_set_zoom(lv_col_header_icon_tempC, 172);

        lv_obj_t * lv_col_header_icon_clock = lv_img_create(lv_col_splitter_3);
        lv_img_set_src(lv_col_header_icon_clock, &img_sand_watch);
        lv_obj_set_width(lv_col_header_icon_clock, LV_SIZE_CONTENT);   /// 48
        lv_obj_set_height(lv_col_header_icon_clock, LV_SIZE_CONTENT);    /// 48
        lv_obj_set_x(lv_col_header_icon_clock, 10);
        lv_obj_set_y(lv_col_header_icon_clock, -6);
        lv_obj_set_align(lv_col_header_icon_clock, LV_ALIGN_TOP_MID);
        lv_obj_add_flag(lv_col_header_icon_clock, LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
        lv_obj_clear_flag(lv_col_header_icon_clock, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
        lv_img_set_zoom(lv_col_header_icon_clock, 172);

        lv_obj_t * lv_list_container = lv_obj_create(lv_task_progress_bar_content_presenter);
        lv_obj_set_width(lv_list_container, 420);
        lv_obj_set_height(lv_list_container, 153);
        lv_obj_set_align(lv_list_container, LV_ALIGN_BOTTOM_MID);
        lv_obj_clear_flag(lv_list_container, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
        lv_obj_set_style_radius(lv_list_container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(lv_list_container, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(lv_list_container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(lv_list_container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t * lv_col_header_label_name = lv_label_create(lv_task_progress_bar_content_presenter);
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

        lv_obj_t * lv_task_progress_bar_lap_line = lv_obj_create(get_container());
        lv_obj_set_width(lv_task_progress_bar_lap_line, 4);
        lv_obj_set_height(lv_task_progress_bar_lap_line, 30);
        lv_obj_set_x(lv_task_progress_bar_lap_line, 0);
        lv_obj_set_y(lv_task_progress_bar_lap_line, 105);
        lv_obj_set_align(lv_task_progress_bar_lap_line, LV_ALIGN_CENTER);
        lv_obj_clear_flag(lv_task_progress_bar_lap_line, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
        lv_obj_set_style_radius(lv_task_progress_bar_lap_line, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(lv_task_progress_bar_lap_line, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(lv_task_progress_bar_lap_line, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(lv_task_progress_bar_lap_line, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

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

        lv_obj_t * lv_list = lv_list_create(lv_task_progress_bar_content_presenter);
        lv_obj_set_width(lv_list, 420);
        lv_obj_set_height(lv_list, 153);
        lv_obj_set_align(lv_list, LV_ALIGN_TOP_MID);
        lv_obj_set_x(lv_list, 0);
        lv_obj_set_y(lv_list, 37);
        lv_obj_set_style_bg_color(lv_list, lv_color_hex(0x77DCDC), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(lv_list, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(lv_list, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_clip_corner(lv_list, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(lv_list, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_left(lv_list, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_right(lv_list, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_top(lv_list, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_bottom(lv_list, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        remember_child_element("[list]", lv_list);
        remember_child_element("[progress_bar]", lv_task_progress_bar_indicator);
        remember_child_element("[header_name]", lv_col_header_label_name);
        remember_child_element("[state_percentage]", lv_task_progress_state_percentage);
        remember_child_element("[state_duration]", lv_task_progress_state_duration);
        
        set_childs_presenter("[list]");
    }

    UITaskListItem * add_task_step(bool clear_childs = false)
    {
        if (clear_childs)
            clear_list();

        _collection.push_back(new UITaskListItem(this));

        if (clear_childs)
            this->focus_on_first_child(false);

        return _collection.back();
    }

    void set_task_header_name(string name)
    {
        lv_label_set_text(get_container_content("[header_name]"), name.c_str());
    }

    void set_task_state_values(double percentage_done, int64_t task_gone_ss, TaskStateEnum state)
    {
        static char buffer[50];

        sprintf(buffer, "%.2f", percentage_done);
        lv_label_set_text(get_container_content("[state_percentage]"), (string(buffer) + "%").c_str());

        sprintf(buffer, "%02d:%02d:%02d", (uint32_t)task_gone_ss / (60 * 60), ((uint32_t)task_gone_ss % (60 * 60)) / 60, (uint32_t)task_gone_ss % 60);
        lv_label_set_text(get_container_content("[state_duration]"), buffer);

        lv_obj_t * bar = get_container_content("[progress_bar]");

        lv_clear_states(bar);
        switch (state)
        {
        case TaskStateEnum::RUNNED: lv_obj_set_state(bar, LV_STATE_USER_1, true); break;
        case TaskStateEnum::PAUSE: lv_obj_set_state(bar, LV_STATE_USER_2, true); break;
        case TaskStateEnum::DONE: lv_obj_set_state(bar, LV_STATE_USER_3, true); break;
        case TaskStateEnum::ERROR: lv_obj_set_state(bar, LV_STATE_USER_4, true); break;
        default: break;
        }

        lv_arc_set_value(bar, percentage_done * 100);
    }

    void update_task_steps_state()
    {
        for (uint8_t i = 0; i < _collection.size(); i++)
            _collection.at(i)->update_ui_context();
    }
};

#endif