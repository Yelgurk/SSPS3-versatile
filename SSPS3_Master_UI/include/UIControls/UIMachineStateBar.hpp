#ifndef UIMachineStateBar_hpp
#define UIMachineStateBar_hpp

#include "../UIElement.hpp"
#include "FRAM_DB.hpp"

enum class ChargeStateEnum : uint8_t { ERROR, STABLE, CHARGERING };

enum class WaterJacketStateEnum : uint8_t { EMPTY, FILLING, FILLED, COOLING };

class UIMachineStateBar : public UIElement
{
private:
    uint32_t bar_height = 0;

public:
    UIMachineStateBar(
        lv_obj_t * lv_screen,
        uint32_t width = 320,
        uint32_t height = 56,
        lv_align_t align = LV_ALIGN_TOP_LEFT,
        int32_t offset_x = 10,
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

#ifndef SSPS3_IS_CHEAP_SOLUTION_YES
        bar_height = height;

        lv_obj_set_width(get_container(), width);
        lv_obj_set_height(get_container(), height);
        lv_obj_align(get_container(), align, offset_x, offset_y);
        lv_obj_remove_flag(get_container(), LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_clip_corner(get_container(), true, 0);
        
        create_splitter(get_container(), 65);
        create_splitter(get_container(), 130);
        create_splitter(get_container(), 225);

        create_state_bar(get_container(), 65, 0, "fan", Translator::get(BAR_RPM), &img_fan);
        create_state_bar(get_container(), 65, 65, "tempC", Translator::get(BAR_TEMPC_MILK), &img_tempC);
        create_state_bar(get_container(), 95, 130, "wJacket", Translator::get(BAR_TEMPC_JACKET), &img_water_hose);
        create_state_bar(
            create_charge_progress_bar(get_container(), 95, 225, "charge_bar"),
            95,
            0,
            "charge",
            Translator::get(BAR_BATTERY),
            &img_charge,
            0
        );

        control_set_values_state_bar(30, true, 85, 85, true, WaterJacketStateEnum::EMPTY, 101, ChargeStateEnum::ERROR);
#else
        bar_height = height;

        lv_obj_set_width(get_container(), 85);
        lv_obj_set_height(get_container(), height);
        lv_obj_align(get_container(), align, offset_x, offset_y);
        lv_obj_remove_flag(get_container(), LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_clip_corner(get_container(), true, 0);

        create_state_bar(get_container(), 65, 10, "tempC", Translator::get(BAR_TEMPC_MILK), &img_tempC);
#endif
    }

    void control_set_values_state_bar(int16_t fan, bool is_fan_flex_speed, int16_t tempC, int16_t tempC_wJ, bool is_wJ_sensor, WaterJacketStateEnum water_jacket_state, int16_t charge_value, ChargeStateEnum charge_state)
    {
#ifndef SSPS3_IS_CHEAP_SOLUTION_YES
        static char buffer[20];
        charge_value = charge_value < 0 ? 0 : (charge_value > 100 ? 100 : charge_value);

        sprintf(buffer, "%d", is_fan_flex_speed ? fan : (fan > 0 ? var_sensor_dac_asyncM_rpm_max.local() : 0));
        lv_label_set_text(get_container_content("[context_fan]"), buffer);
        sprintf(buffer, "%d", tempC);
        lv_label_set_text(get_container_content("[context_tempC]"), buffer);

        if (!is_wJ_sensor)
        {
            lv_obj_set_style_text_font(get_container_content("[context_wJacket]"), &OpenSans_bold_20px, LV_PART_MAIN | LV_STATE_DEFAULT);
            switch (water_jacket_state)
            {
                case WaterJacketStateEnum::FILLING: lv_label_set_text(get_container_content("[context_wJacket]"), Translator::get(JACKET_FILLING).c_str()); break;
                case WaterJacketStateEnum::FILLED: lv_label_set_text(get_container_content("[context_wJacket]"), Translator::get(JACKET_DONE).c_str()); break;
                case WaterJacketStateEnum::COOLING: lv_label_set_text(get_container_content("[context_wJacket]"), Translator::get(JACKET_COOLING).c_str()); break;
                default: lv_label_set_text(get_container_content("[context_wJacket]"), Translator::get(JACKET_EMPTY).c_str()); break;
            }
        }
        else
        {
            lv_obj_set_style_text_font(get_container_content("[context_wJacket]"), &OpenSans_bold_16px, LV_PART_MAIN | LV_STATE_DEFAULT);
            string wj_tempC_str = "";

            switch (water_jacket_state)
            {
                case WaterJacketStateEnum::FILLING: wj_tempC_str = Translator::get(JACKET_FILLING); break;
                case WaterJacketStateEnum::FILLED:  wj_tempC_str = Translator::get(JACKET_DONE); break;
                case WaterJacketStateEnum::COOLING: wj_tempC_str = Translator::get(JACKET_COOLING); break;
                default: wj_tempC_str = Translator::get(JACKET_EMPTY); break;
            }

            wj_tempC_str += ", ";
            lv_label_set_text(get_container_content("[context_wJacket]"), (wj_tempC_str + to_string(tempC_wJ) + "°").c_str());
        }

        lv_obj_t * bar_ptr = get_container_content("[context_charge_bar]");

        lv_clear_states(bar_ptr);

        switch (charge_state)
        {
        case ChargeStateEnum::CHARGERING: {
            lv_label_set_text(get_container_content("[context_charge]"), Translator::get(BAT_CHARGERING).c_str());
            }; break;
        
        case ChargeStateEnum::STABLE: {
            lv_label_set_text(get_container_content("[context_charge]"), (to_string(charge_value) + "%").c_str());
            if (charge_value > 70)
                lv_obj_set_state(bar_ptr, LV_STATE_USER_1, true);
            else
            if (charge_value > 30)
                lv_obj_set_state(bar_ptr, LV_STATE_USER_2, true);
            else
            if (charge_value <= 30)
                lv_obj_set_state(bar_ptr, LV_STATE_USER_3, true);

            lv_bar_set_value(bar_ptr, charge_value, LV_ANIM_OFF);
            }; break;
        
        default: {
            lv_obj_set_state(bar_ptr, LV_STATE_USER_3, true);
            lv_label_set_text(get_container_content("[context_charge]"), Translator::get(BAT_ERROR).c_str());
            lv_bar_set_value(bar_ptr, 100, LV_ANIM_OFF);
            }; break;
        }
#endif
    }

private:
    void create_splitter(lv_obj_t * bar, int32_t margin_left)
    {
        lv_obj_t * splitter = lv_obj_create(bar);
        lv_obj_set_width(splitter, 1);
        lv_obj_set_height(splitter, 36);
        lv_obj_align(splitter, LV_ALIGN_LEFT_MID, margin_left, 0);
        lv_obj_set_style_border_width(splitter, 0, 0);
        lv_obj_set_style_radius(splitter, 0, 0);
        lv_obj_set_style_bg_color(splitter, COLOR_DARK_GREY, 0);
    }
    
    void create_state_bar(lv_obj_t * bar, int32_t width, int32_t margin, string name, const string header_demo, const lv_img_dsc_t* image_src_ptr, int16_t img_left_margin = -10)
    {
        lv_obj_t * state_container = lv_obj_create(bar);
        lv_obj_set_width(state_container, width);
        lv_obj_set_height(state_container, bar_height);
        lv_obj_align(state_container, LV_ALIGN_LEFT_MID, margin, 5);
        lv_obj_set_style_bg_opa(state_container, 0, 0);
        lv_obj_set_style_radius(state_container, 0, 0);
        lv_obj_set_style_border_width(state_container, 0, 0);
        lv_obj_set_style_pad_all(state_container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_remove_flag(state_container, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t * lv_base_text = lv_label_create(state_container);
        lv_obj_set_style_text_font(lv_base_text, &OpenSans_bold_16px, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text(lv_base_text, header_demo.c_str());
        lv_obj_set_align(lv_base_text, LV_ALIGN_TOP_MID);

        lv_obj_t * lv_context_aligner = lv_obj_create(state_container);
        lv_obj_remove_flag(lv_context_aligner, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_align(lv_context_aligner, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_obj_set_style_pad_all(lv_context_aligner, 0, 0);
        lv_obj_set_style_pad_left(lv_context_aligner, 24, 0);
        lv_obj_set_style_pad_right(lv_context_aligner, 4, 0);
        lv_obj_set_style_border_width(lv_context_aligner, 0, 0);
        lv_obj_set_style_bg_opa(lv_context_aligner, 0, 0);
        lv_obj_set_style_radius(lv_context_aligner, 0, 0);
        lv_obj_set_width(lv_context_aligner, LV_SIZE_CONTENT);
        lv_obj_set_height(lv_context_aligner, LV_SIZE_CONTENT);
        
        lv_obj_t * lv_context_text = lv_label_create(lv_context_aligner);
        lv_obj_set_style_text_font(lv_context_text, &OpenSans_regular_20px, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text(lv_context_text, "123");
        lv_obj_set_align(lv_context_text, LV_ALIGN_CENTER);

        lv_obj_t * lv_setter_icon = lv_image_create(lv_context_aligner);
        lv_image_set_src(lv_setter_icon, image_src_ptr);
        lv_obj_set_style_pad_all(lv_setter_icon, 0, 0);
        lv_obj_align(lv_setter_icon, LV_ALIGN_LEFT_MID, -36, 0);
        lv_obj_set_width(lv_setter_icon, LV_SIZE_CONTENT);
        lv_obj_set_height(lv_setter_icon, LV_SIZE_CONTENT);
        lv_obj_add_flag(lv_setter_icon, LV_OBJ_FLAG_ADV_HITTEST);
        lv_obj_remove_flag(lv_setter_icon, LV_OBJ_FLAG_SCROLLABLE);
        lv_image_set_scale(lv_setter_icon, 104);

        remember_child_element("[base_" + name + "]", lv_base_text);
        remember_child_element("[context_" + name + "]", lv_context_text);
    }

    lv_obj_t * create_charge_progress_bar(lv_obj_t * bar, int32_t width, int32_t margin, string name)
    {
        lv_obj_t * progress_bar = lv_bar_create(bar);
        lv_bar_set_value(progress_bar, 40, LV_ANIM_ON);
        lv_obj_set_width(progress_bar, width);
        lv_obj_set_height(progress_bar, bar_height);
        lv_obj_align(progress_bar, LV_ALIGN_LEFT_MID, margin, 0);
        lv_obj_set_style_radius(progress_bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(progress_bar, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(progress_bar, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(progress_bar, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);

        lv_obj_set_style_bg_color(progress_bar, COLOR_SKY_BLUE, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(progress_bar, COLOR_SKY_BLUE, LV_PART_INDICATOR | LV_STATE_DEFAULT);
        
        lv_obj_set_style_bg_color(progress_bar, COLOR_GREEN, LV_PART_MAIN | LV_STATE_USER_1);
        lv_obj_set_style_bg_color(progress_bar, COLOR_GREEN_MEDIUM, LV_PART_INDICATOR | LV_STATE_USER_1);
        
        lv_obj_set_style_bg_color(progress_bar, COLOR_YELLOW_SMOKE, LV_PART_MAIN | LV_STATE_USER_2);
        lv_obj_set_style_bg_color(progress_bar, COLOR_YELLOW, LV_PART_INDICATOR | LV_STATE_USER_2);
        
        lv_obj_set_style_bg_color(progress_bar, COLOR_PINK, LV_PART_MAIN | LV_STATE_USER_3);
        lv_obj_set_style_bg_color(progress_bar, COLOR_RED, LV_PART_INDICATOR | LV_STATE_USER_3);

        remember_child_element("[context_" + name + "]", progress_bar);
        return progress_bar;
    }
};

#endif