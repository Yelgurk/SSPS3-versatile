#ifndef UIValueSetter_hpp
#define UIValueSetter_hpp

#include "../UIElement.hpp"


struct ValueSetterFuctionContainer
{
private:
    bool _is_init = false;

public:
    UIAction var_inc;
    UIAction var_dec;
    UIAction var_accept;

    ValueSetterFuctionContainer() {}

    ValueSetterFuctionContainer(
        UIAction var_inc,
        UIAction var_dec,
        UIAction var_accept):
    var_inc(var_inc),
    var_dec(var_dec),
    var_accept(var_accept)
    {
        _is_init = true;
    }

    bool is_init() {
        return _is_init;
    }
};

class UIValueSetter : public UIElement
{
private:
    ValueSetterFuctionContainer var_edit_func;
    uint16_t height_setter_container = 240;
    bool is_top_column_setter = false;

public:
    UIValueSetter(
        UIElement * parent_navi,
        int32_t width,
        bool top_column_setter = false,
        string header_text = "",
        const lv_image_dsc_t * header_icon = nullptr
    ) : UIValueSetter(
        parent_navi,
        0,
        width,
        0,
        0,
        top_column_setter,
        header_text,
        header_icon
    ) {}

    UIValueSetter(
        UIElement * parent_navi,
        int32_t page,
        int32_t width = 0,
        int32_t offset_x = 0,
        int32_t offset_y = 0,
        bool top_column_setter = false,
        string header_text = "",
        const lv_image_dsc_t * header_icon = nullptr,
        bool is_button = false
    ) : UIElement {
        { EquipmentType::All },
        {},
        true,
        false,
        false,
        PlaceControlIn::Control,
        parent_navi->get_screen(),
        parent_navi,
        { StyleActivator::Focus, StyleActivator::Unscrollable, StyleActivator::Shadow },
        false,
        top_column_setter ? -offset_y : 0
    }
    {
        this->is_top_column_setter = top_column_setter;
        this->set_position(page, offset_x, offset_y);
        
        lv_obj_set_width(get_container(), width == 0 ? LV_SIZE_CONTENT : width);
        lv_obj_set_style_min_width(get_container(), 40, 0);
        lv_obj_set_height(get_container(), is_button ? 50 : 90);
        lv_obj_set_style_bg_color(get_container(), COLOR_GREY, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_all(get_container(), 5, 0);
        lv_obj_set_style_pad_all(get_container(), 1, LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_set_style_bg_color(get_container(), COLOR_YELLOW, LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_set_style_pad_all(get_container(), 0, LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_set_style_border_width(get_container(), 0, LV_PART_MAIN | LV_STATE_FOCUSED);

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

        if (!is_button)
        {
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
        }

        if (header_icon == nullptr || is_button)
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

        if (is_button)
        {
            lv_obj_align(label_setter_header, LV_ALIGN_CENTER, 0, 0);
            lv_label_set_text(label_setter_value, "");
        }

        remember_child_element("[header]", label_setter_header);
        remember_child_element("[value]", label_setter_value);

        update_ui_base();
        update_ui_context();
    }

    void set_position(int32_t page, int32_t offset_x, int32_t offset_y, int32_t width = -1)
    {
        _focus_offset_y = is_top_column_setter ? -offset_y : 0;
        page = page < 0 ? 0 : page;
        offset_y = offset_y + (page * height_setter_container);
        
        lv_obj_align(get_container(), LV_ALIGN_TOP_LEFT, offset_x, offset_y);
        if (width >= 0)
            lv_obj_set_width(get_container(), width);
    }

    void set_extra_button_logic(ValueSetterFuctionContainer var_edit_func)
    {
        this->var_edit_func = var_edit_func;

        set_key_press_actions({
            KeyModel(KeyMap::TOP,       [this]() { this->var_edit_func.var_inc(); this->update_ui_context(); }),
            KeyModel(KeyMap::BOTTOM,    [this]() { this->var_edit_func.var_dec(); this->update_ui_context(); }),
            KeyModel(KeyMap::LEFT_BOT,  [this]() { this->var_edit_func.var_accept(); this->update_ui_context(); }),
            KeyModel(KeyMap::LEFT,      [this]() { this->navi_prev(); }),
            KeyModel(KeyMap::RIGHT,     [this]() { this->navi_next(); }),
            KeyModel(KeyMap::LEFT_TOP,  [this]() { this->navi_back(); })
        });
    }

    void set_header_text(string new_header) {
        lv_label_set_text(get_container_content("[header]"), new_header.c_str());
    }

    void set_value(int32_t new_value) {
        set_value(to_string(new_value));
    }

    void set_value(double new_value) {
        set_value(to_string(new_value));
    }

    void set_value(string new_value) {
        lv_label_set_text(get_container_content("[value]"), new_value.c_str());
    }
};

#endif