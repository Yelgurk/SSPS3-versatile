#ifndef UIBlowValListItem_hpp
#define UIBlowValListItem_hpp

#include "../UIElement.hpp"

class UIBlowValListItem : public UIElement
{
public:
    UIBlowValListItem(UIElement * parent_navi, vector<KeyModel> key_press_actions)
    : UIElement
    {
        { EquipmentType::All },
        key_press_actions,
        true,
        false,
        false,
        PlaceControlIn::Control,
        parent_navi->get_screen(),
        parent_navi,
        { StyleActivator::Unscrollable, StyleActivator::Rectangle, StyleActivator::Focus }
    }
    {
        lv_obj_set_width(get_container(), 420);
        lv_obj_set_height(get_container(), 30);
        lv_obj_set_style_bg_opa(get_container(), 185, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(get_container(), COLOR_WHITE, 0);
        lv_obj_set_style_bg_color(get_container(), COLOR_GREEN, LV_PART_MAIN | LV_STATE_USER_1);
        lv_obj_set_style_bg_color(get_container(), COLOR_YELLOW, LV_PART_MAIN | LV_STATE_USER_2);
        lv_obj_set_style_bg_color(get_container(), COLOR_BLUE, LV_PART_MAIN | LV_STATE_USER_3);
        lv_obj_set_style_bg_color(get_container(), COLOR_RED, LV_PART_MAIN | LV_STATE_USER_4);

        /*
        header = lv_label_create(get_container());
        lv_label_set_long_mode(header, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_set_flex_grow(header, 1);
        lv_obj_set_width(header, LV_SIZE_CONTENT);   /// 1
        lv_obj_set_style_max_width(header, 210, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_height(header, LV_SIZE_CONTENT);
        lv_obj_set_align(header, LV_ALIGN_LEFT_MID);
        lv_obj_set_x(header, 0);
        lv_obj_set_style_pad_left(header, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(header, &OpenSans_bold_20px, LV_PART_MAIN | LV_STATE_DEFAULT);


        lab_1 = lv_label_create(get_container());
        lv_obj_set_width(lab_1, LV_SIZE_CONTENT);
        lv_obj_set_height(lab_1, LV_SIZE_CONTENT);
        lv_obj_set_align(lab_1, LV_ALIGN_CENTER);
        lv_obj_set_x(lab_1, 25);
        lv_label_set_text(lab_1, string("13").c_str());
        lv_obj_set_style_text_font(lab_1, &OpenSans_bold_20px, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_opa(lab_1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_opa(lab_1, 0, LV_PART_MAIN | LV_STATE_USER_1);

        lab_2 = lv_label_create(get_container());
        lv_obj_set_width(lab_2, LV_SIZE_CONTENT);
        lv_obj_set_height(lab_2, LV_SIZE_CONTENT);
        lv_obj_set_align(lab_2, LV_ALIGN_CENTER);
        lv_obj_set_x(lab_2, 75);
        lv_label_set_text(lab_2, string("65").c_str());
        lv_obj_set_style_text_font(lab_2, &OpenSans_bold_20px, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_opa(lab_2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_opa(lab_2, 0, LV_PART_MAIN | LV_STATE_USER_1);

        lab_3 = lv_label_create(get_container());
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

        add_ui_base_action([this](){
            lv_label_set_text(this->header, this->node->name.c_str());
        });
        
        add_ui_context_action([this]() {
            lv_label_set_text(this->lab_1, to_string(this->node->rotation).c_str());
            lv_label_set_text(this->lab_2, to_string(this->node->tempC).c_str());
            lv_label_set_text(this->lab_3, to_string(this->node->duratTotalSS).c_str());
        });

        update_ui_base();
        update_ui_context();
        */
    }
};

#endif