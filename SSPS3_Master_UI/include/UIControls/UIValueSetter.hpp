#ifndef UIValueSetter_hpp
#define UIValueSetter_hpp

#include "../UIElement.hpp"

class UIValueSetter : public UIElement
{
private:
    UIAction var_inc;
    UIAction var_dec;
    UIAction setter_set_header;
    UIAction setter_set_value;

public:
    UIValueSetter(
        UIElement * parent_navi,
        lv_align_t align,
        int32_t page,
        int32_t width = 0,
        int32_t offset_x = 0,
        int32_t offset_y = 0,
        bool top_column_setter = false,
        string header_text = "",
        const lv_image_dsc_t * header_icon = nullptr,
        UIAction var_inc = NULL,
        UIAction var_dec = NULL,
        UIAction setter_set_header = NULL,
        UIAction setter_set_value = NULL
    ) : UIElement {
        { EquipmentType::All },
        {},
        true,
        true,
        false,
        PlaceControlIn::Control,
        parent_navi->get_screen(),
        parent_navi,
        { StyleActivator::Focus, StyleActivator::Select, StyleActivator::Unscrollable, StyleActivator::Shadow },
        false,
        top_column_setter ? -offset_y : 0
    }
    {
        if (var_inc != NULL)
            this->var_inc = var_inc;
        if (var_dec != NULL)
            this->var_dec = var_dec;
        if (setter_set_header != NULL)
            this->setter_set_header = setter_set_header;
        if (setter_set_value != NULL)
            this->setter_set_value = setter_set_value;

        set_key_press_actions({
            KeyModel(KeyMap::TOP,
            [this]() {
                if (this->var_inc) this->var_inc();
                if (this->setter_set_value) this->setter_set_value();
            }),
            KeyModel(KeyMap::BOTTOM,
            [this]() {
                if (this->var_dec) this->var_dec();
                if (this->setter_set_value) this->setter_set_value();
            }),
            KeyModel(KeyMap::LEFT,      [this]() { this->navi_prev(); }),
            KeyModel(KeyMap::RIGHT,     [this]() { this->navi_next(); }),
            KeyModel(KeyMap::LEFT_TOP,  [this]() { this->navi_back(); })
        });

        static uint16_t height_setter_container = 240;
        page = page < 0 ? 0 : page;
        offset_y = offset_y + (page * height_setter_container);

        lv_obj_set_width(get_container(), width == 0 ? LV_SIZE_CONTENT : width);
        lv_obj_set_style_min_width(get_container(), 40, 0);
        lv_obj_set_height(get_container(), 90);
        lv_obj_align(get_container(), align, offset_x, offset_y);
        lv_obj_set_style_bg_color(get_container(), COLOR_GREY, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_all(get_container(), 5, 0);
        lv_obj_set_style_pad_all(get_container(), 1, LV_PART_MAIN | LV_STATE_FOCUSED);

        lv_obj_t * label_setter_header = lv_label_create(get_container());
        lv_obj_align(label_setter_header, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_set_style_text_font(label_setter_header, &OpenSans_bold_20px, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text(label_setter_header, header_text.c_str());

        lv_obj_t * icon_setter_header = lv_image_create(get_container());
        lv_obj_align(icon_setter_header, LV_ALIGN_TOP_MID, 0, -12);
        lv_obj_set_width(icon_setter_header, LV_SIZE_CONTENT);
        lv_obj_set_height(icon_setter_header, LV_SIZE_CONTENT);
        lv_image_set_scale(icon_setter_header, 128);
        if (header_icon != nullptr)
            lv_image_set_src(icon_setter_header, header_icon);

        lv_obj_t * icon_setter_up = lv_image_create(get_container());
        lv_obj_align(icon_setter_up, LV_ALIGN_TOP_MID, 0, 8);
        lv_obj_set_width(icon_setter_up, LV_SIZE_CONTENT);
        lv_obj_set_height(icon_setter_up, LV_SIZE_CONTENT);
        lv_image_set_src(icon_setter_up, &img_arrow_up);
        lv_image_set_scale(icon_setter_up, 160);

        lv_obj_t * icon_setter_down = lv_image_create(get_container());
        lv_obj_align(icon_setter_down, LV_ALIGN_BOTTOM_MID, 0, 16);
        lv_obj_set_width(icon_setter_down, LV_SIZE_CONTENT);
        lv_obj_set_height(icon_setter_down, LV_SIZE_CONTENT);
        lv_image_set_src(icon_setter_down, &img_arrow_down);
        lv_image_set_scale(icon_setter_down, 160);

        if (header_icon == nullptr)
        {
            lv_obj_set_width(icon_setter_header, 0);
            lv_obj_set_height(icon_setter_header, 0);
            lv_img_set_zoom(icon_setter_header, 0);
        }
        else
            lv_obj_set_style_opa(label_setter_header, 0, 0);

        lv_obj_t * label_setter_value = lv_label_create(get_container());
        lv_obj_align(label_setter_value, LV_ALIGN_CENTER, 0, 12);
        lv_obj_set_style_text_font(label_setter_value, &OpenSans_bold_20px, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text(label_setter_value, "Val #");

        remember_child_element("[header]", label_setter_header);
        remember_child_element("[value]", label_setter_value);
    }

    void set_header_text(string new_header) {
        lv_label_set_text(get_container_content("[header]"), new_header.c_str());
    }

    void set_value(string new_value) {
        lv_label_set_text(get_container_content("[value]"), new_value.c_str());
    }
};

#endif