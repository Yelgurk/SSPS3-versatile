#ifndef UITaskListItem_hpp
#define UITaskListItem_hpp

#include "../UIElement.hpp"

enum class StepStateEnum : uint8_t { AWAIT, RUNNED, PAUSE, DONE, ERROR };

struct EditFuctionContainer
{
private:
    bool _is_init = false;

public:
    UIAction var_inc_fan;
    UIAction var_dec_fan;
    UIAction var_inc_tempC;
    UIAction var_dec_tempC;
    UIAction var_inc_durat;
    UIAction var_dec_durat;

    EditFuctionContainer() {}

    EditFuctionContainer(
        UIAction var_inc_fan,
        UIAction var_dec_fan,
        UIAction var_inc_tempC,
        UIAction var_dec_tempC,
        UIAction var_inc_durat,
        UIAction var_dec_durat):
    var_inc_fan(var_inc_fan),
    var_dec_fan(var_dec_fan),
    var_inc_tempC(var_inc_tempC),
    var_dec_tempC(var_dec_tempC),
    var_inc_durat(var_inc_durat),
    var_dec_durat(var_dec_durat)
    {
        _is_init = true;
    }

    bool is_init() {
        return _is_init;
    }
};

class UITaskListItem : public UIElement
{
private:
    EditFuctionContainer var_edit_func;

public:
    UITaskListItem(UIElement * parent_navi)
    : UIElement
    {
        { EquipmentType::All },
        {},
        true,
        false,
        false,
        PlaceControlIn::Control,
        parent_navi->get_screen(),
        parent_navi,
        { StyleActivator::Unscrollable, StyleActivator::Rectangle, StyleActivator::Focus },
        false,
        0,
        true
    }
    {
        lv_obj_set_width(get_container(), 420);
        lv_obj_set_height(get_container(), 30);
        lv_obj_set_style_bg_opa(get_container(), 220, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(get_container(), COLOR_WHITE, 0);
        lv_obj_set_style_bg_color(get_container(), COLOR_BLUE, LV_PART_MAIN | LV_STATE_EDITED);
        lv_obj_set_style_bg_color(get_container(), COLOR_YELLOW, LV_PART_MAIN | LV_STATE_USER_1);
        lv_obj_set_style_bg_color(get_container(), COLOR_GREEN, LV_PART_MAIN | LV_STATE_USER_2);
        lv_obj_set_style_bg_color(get_container(), COLOR_RED, LV_PART_MAIN | LV_STATE_USER_3);

        lv_obj_set_style_opa(get_container(), 255, 0);
        lv_obj_set_style_opa(get_container(), 255, LV_STATE_EDITED);
        lv_obj_set_style_opa(get_container(), 255, LV_STATE_USER_1);
        lv_obj_set_style_opa(get_container(), 255, LV_STATE_USER_2);
        lv_obj_set_style_opa(get_container(), 255, LV_STATE_USER_3);
        lv_obj_set_style_opa(get_container(), 255, LV_STATE_USER_4);
        lv_obj_set_style_bg_opa(get_container(), 220, 0);
        lv_obj_set_style_bg_opa(get_container(), 220, LV_PART_MAIN | LV_STATE_EDITED);
        lv_obj_set_style_bg_opa(get_container(), 220, LV_PART_MAIN | LV_STATE_USER_1);
        lv_obj_set_style_bg_opa(get_container(), 220, LV_PART_MAIN | LV_STATE_USER_2);
        lv_obj_set_style_bg_opa(get_container(), 220, LV_PART_MAIN | LV_STATE_USER_3);
        lv_obj_set_style_bg_opa(get_container(), 220, LV_PART_MAIN | LV_STATE_USER_4);

        lv_obj_t * header = lv_label_create(get_container());
        lv_label_set_long_mode(header, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_set_flex_grow(header, 1);
        lv_obj_set_width(header, LV_SIZE_CONTENT);   /// 1
        lv_obj_set_style_max_width(header, 210, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_height(header, LV_SIZE_CONTENT);
        lv_obj_set_align(header, LV_ALIGN_LEFT_MID);
        lv_obj_set_x(header, 0);
        lv_obj_set_style_pad_left(header, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(header, &OpenSans_bold_20px, LV_PART_MAIN | LV_STATE_DEFAULT);


        lv_obj_t * lab_1 = lv_label_create(get_container());
        lv_obj_set_width(lab_1, LV_SIZE_CONTENT);
        lv_obj_set_height(lab_1, LV_SIZE_CONTENT);
        lv_obj_set_align(lab_1, LV_ALIGN_CENTER);
        lv_obj_set_x(lab_1, 25);
        lv_label_set_text(lab_1, string("13").c_str());
        lv_obj_set_style_text_font(lab_1, &OpenSans_bold_20px, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_opa(lab_1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_opa(lab_1, 0, LV_PART_MAIN | LV_STATE_USER_1);

        lv_obj_t * lab_2 = lv_label_create(get_container());
        lv_obj_set_width(lab_2, LV_SIZE_CONTENT);
        lv_obj_set_height(lab_2, LV_SIZE_CONTENT);
        lv_obj_set_align(lab_2, LV_ALIGN_CENTER);
        lv_obj_set_x(lab_2, 75);
        lv_label_set_text(lab_2, string("65").c_str());
        lv_obj_set_style_text_font(lab_2, &OpenSans_bold_20px, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_opa(lab_2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_opa(lab_2, 0, LV_PART_MAIN | LV_STATE_USER_1);

        lv_obj_t * lab_3 = lv_label_create(get_container());
        lv_obj_set_width(lab_3, LV_SIZE_CONTENT);
        lv_obj_set_height(lab_3, LV_SIZE_CONTENT);
        lv_obj_set_align(lab_3, LV_ALIGN_CENTER);
        lv_obj_set_x(lab_3, 155);
        lv_label_set_text(lab_3, string("128м 77с").c_str());
        lv_obj_set_style_text_font(lab_3, &OpenSans_bold_20px, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_opa(lab_3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_opa(lab_3, 0, LV_PART_MAIN | LV_STATE_USER_1);

        remember_child_element("[header]", header);
        remember_child_element("[val1]", lab_1);
        remember_child_element("[val2]", lab_2);
        remember_child_element("[val3]", lab_3);

        update_ui_base();
        update_ui_context();
    }

    void set_extra_button_logic(EditFuctionContainer var_edit_func)
    {
        this->var_edit_func = var_edit_func;

        set_key_press_actions({
            KeyModel(KeyMap::L_STACK_4, [this]() { this->var_edit_func.var_inc_fan(); this->update_ui_context(); }),
            KeyModel(KeyMap::L_STACK_3, [this]() { this->var_edit_func.var_dec_fan(); this->update_ui_context(); }),
            KeyModel(KeyMap::R_STACK_4, [this]() { this->var_edit_func.var_inc_durat(); this->update_ui_context(); }),
            KeyModel(KeyMap::R_STACK_3, [this]() { this->var_edit_func.var_dec_durat(); this->update_ui_context(); }),
            KeyModel(KeyMap::R_STACK_2, [this]() { this->var_edit_func.var_inc_tempC(); this->update_ui_context(); }),
            KeyModel(KeyMap::R_STACK_1, [this]() { this->var_edit_func.var_dec_tempC(); this->update_ui_context(); })
        });
    }

    void set_step_name(string task_step_name)
    {
        lv_label_set_text(get_container_content("[header]"), task_step_name.c_str());
    }

    void set_step_values(int32_t val_fan, bool is_fan_flex_speed, int32_t val_tempC, int32_t val_durat, StepStateEnum state)
    {
        static char buffer[50];

        sprintf(buffer, "%02d:%02d", (uint32_t)abs(val_durat) / 60, (uint32_t)abs(val_durat) % 60);

        if (is_fan_flex_speed)
            lv_label_set_text(get_container_content("[val1]"), to_string(val_fan).c_str());
        else
            lv_label_set_text(get_container_content("[val1]"), (val_fan > 0 ? Translator::get(YES) : Translator::get(NO)).c_str());
        lv_label_set_text(get_container_content("[val2]"), to_string(val_tempC).c_str());
        lv_label_set_text(get_container_content("[val3]"), val_durat == 0 ? "" : ((val_durat < 0 ? "-" : "") + string(buffer)).c_str());

        if (!lv_obj_has_state(get_container(), LV_STATE_EDITED) && state == StepStateEnum::RUNNED)
            lv_obj_scroll_to_view(get_container(), LV_ANIM_ON);

        lv_obj_set_state(get_container(), LV_STATE_EDITED, false);
        lv_obj_set_state(get_container(), LV_STATE_USER_1, false);
        lv_obj_set_state(get_container(), LV_STATE_USER_2, false);
        lv_obj_set_state(get_container(), LV_STATE_USER_3, false);

        switch (state)
        {
        case StepStateEnum::RUNNED: lv_obj_set_state(get_container(), LV_STATE_EDITED, true); break;
        case StepStateEnum::PAUSE: lv_obj_set_state(get_container(), LV_STATE_USER_1, true); break;
        case StepStateEnum::DONE: lv_obj_set_state(get_container(), LV_STATE_USER_2, true); break;
        case StepStateEnum::ERROR: lv_obj_set_state(get_container(), LV_STATE_USER_3, true); break;
        default: break;
        }
    }
};

#endif