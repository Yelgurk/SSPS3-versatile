#ifndef UIMenuListItem_hpp
#define UIMenuListItem_hpp

#include "../UIElement.hpp"

class UIMenuListItem : public UIElement
{
private:
    const uint16_t height_selector = 40;
    const uint16_t width_selector = 160;
    const uint16_t height_setter_container = 240;
    const uint16_t width_setter_container = 295;

public:
    UIMenuListItem(UIElement * parent_navi, string name = "Настройка", bool create_childs_presenter = true)
    : UIElement
    {
        { EquipmentType::All },
        {
            KeyModel(KeyMap::LEFT,      [this]() { this->navi_prev(); }),
            KeyModel(KeyMap::RIGHT,     [this]() { this->navi_next(); }),
            KeyModel(KeyMap::LEFT_TOP,  [this]() { this->navi_back(); }),
            KeyModel(KeyMap::LEFT_BOT,  [this]() { this->navi_ok(); })
        },
        true,
        true,
        false,
        PlaceControlIn::Control,
        parent_navi->get_screen(),
        parent_navi,
        { StyleActivator::Unscrollable, StyleActivator::Rectangle, StyleActivator::Focus, StyleActivator::Select },
        true
    }
    {
        lv_obj_set_height(get_container(), height_selector);
        lv_obj_set_width(get_container(), width_selector);

        lv_obj_t * label_name = lv_label_create(get_container());
        lv_obj_set_flex_grow(label_name, 1);
        lv_obj_set_width(label_name, LV_SIZE_CONTENT);
        lv_obj_set_height(label_name, LV_SIZE_CONTENT);
        lv_obj_set_style_max_width(label_name, width_selector - 20, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_align(label_name, LV_ALIGN_LEFT_MID, 10, 0);
        lv_label_set_long_mode(label_name, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_set_style_text_font(label_name, &OpenSans_bold_20px, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text(label_name, name.c_str());

        if (create_childs_presenter)
        {
            lv_obj_t * setters_container = lv_obj_create(parent_navi->get_screen());
            lv_obj_set_style_pad_all(setters_container, 0, 0);
            lv_obj_set_width(setters_container, width_setter_container);
            lv_obj_set_height(setters_container, height_setter_container);
            lv_obj_align(setters_container, LV_ALIGN_RIGHT_MID, -10, 30);
            lv_obj_set_style_radius(setters_container, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(setters_container, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(setters_container, COLOR_WHITE, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_clip_corner(setters_container, true, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(setters_container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_color(setters_container, COLOR_MEDIUM_GREY, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(setters_container, 155, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_width(setters_container, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_spread(setters_container, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_offset_x(setters_container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_offset_y(setters_container, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

            remember_child_element("[settings_container]", setters_container);
            set_childs_presenter("[settings_container]");
        }

        remember_child_element("[settings_name]", label_name);
        set_childs_hidden();
    }

    void set_page_header(string page_header, int32_t page)
    {
        page = page < 0 ? 0 : page;

        lv_obj_t * header_label = lv_label_create(get_navi_childs_presenter());
        lv_obj_align(header_label, LV_ALIGN_TOP_LEFT, 10, 5 + height_setter_container * page);
        lv_obj_set_style_text_font(header_label, &OpenSans_bold_24px, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text(header_label, page_header.c_str());

        remember_child_element("[page_header_" + to_string(page) + "]", header_label);
    }
};

#endif