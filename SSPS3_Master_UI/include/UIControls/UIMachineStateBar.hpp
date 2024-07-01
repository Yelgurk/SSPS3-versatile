#ifndef UIMachineStateBar_hpp
#define UIMachineStateBar_hpp

#include "../UIElement.hpp"

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
        bar_height = height;

        lv_obj_set_width(get_container(), width);
        lv_obj_set_height(get_container(), height);
        lv_obj_align(get_container(), align, offset_x, offset_y);
        lv_obj_remove_flag(get_container(), LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_clip_corner(get_container(), true, 0);
        
        create_splitter(get_container(), 65);
        create_splitter(get_container(), 130);
        create_splitter(get_container(), 225);

        create_state_bar(get_container(), 65, 0, "fan", "об/мин", &img_fan);
        create_state_bar(get_container(), 65, 65, "tempC", "вода °C", &img_tempC);
        create_state_bar(get_container(), 95, 130, "wJacket", "рубашка", &img_water_hose);
        create_state_bar(get_container(), 95, 225, "charge", "батарея", &img_charge, 0);

        control_set_values_state_bar(30, 85, 2, 101);
    }

    void control_set_values_state_bar(int16_t state_fan, int16_t state_tempC, uint8_t state_water_in_jacket, int16_t state_charge)
    {
        static char buffer[20];

        sprintf(buffer, "%d", state_fan);
        lv_label_set_text(get_container_content("[context_fan]"), buffer);

        sprintf(buffer, "%d", state_tempC);
        lv_label_set_text(get_container_content("[context_tempC]"), buffer);

        lv_label_set_text(get_container_content("[context_wJacket]"), (state_water_in_jacket > 1 ? "набор" : (string)(state_water_in_jacket == 1 ? "заполн." : "пусто")).c_str());

        lv_label_set_text(get_container_content("[context_charge]"), state_charge > 100 ? "зарядка" : (to_string(state_charge) + "%").c_str());
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
};

#endif