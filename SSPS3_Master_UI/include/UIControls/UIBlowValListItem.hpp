#ifndef UIBlowValListItem_hpp
#define UIBlowValListItem_hpp

#include "../UIElement.hpp"

enum class BlowVarTypeEnum : uint8_t { NONE, LITRES, TIMER };

class UIBlowValListItem : public UIElement
{
private:
    const int16_t width = 210;
    const int16_t height = 50;
    BlowVarTypeEnum var_type = BlowVarTypeEnum::NONE;

    UIAction var_inc;
    UIAction var_dec;
    UIAction setter_set_value;
    UIAction task_start;

public:
    UIBlowValListItem(
        UIElement * parent_navi,
        BlowVarTypeEnum type,
        int32_t value = 5000,
        UIAction var_inc = NULL,
        UIAction var_dec = NULL,
        UIAction setter_set_value = NULL,
        UIAction task_start = NULL
    )
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
        { StyleActivator::Unscrollable, StyleActivator::Rectangle, StyleActivator::Focus }
    }
    {
        this->var_type = type;

        if (setter_set_value != NULL)
        {
            this->setter_set_value = setter_set_value;
            add_ui_context_action(setter_set_value);
        }
        if (var_inc != NULL)
            this->var_inc = var_inc;
        if (var_dec != NULL)
            this->var_dec = var_dec;
        if (task_start != NULL)
            this->task_start = task_start;

        set_key_press_actions({
            KeyModel(KeyMap::LEFT,
            [this]() {
                if (this->var_inc) this->var_inc();
                if (this->setter_set_value) this->setter_set_value();
            }),
            KeyModel(KeyMap::RIGHT,
            [this]() {
                if (this->var_dec) this->var_dec();
                if (this->setter_set_value) this->setter_set_value();
            }),
            KeyModel(KeyMap::RIGHT,
            [this]() {
                if (this->task_start) this->task_start();
            })
        });

        lv_obj_set_width(get_container(), width);
        lv_obj_set_height(get_container(), height);
        lv_obj_set_style_bg_opa(get_container(), 185, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(get_container(), COLOR_WHITE, 0);
        lv_obj_set_style_border_opa(get_container(), 0, 0);
        lv_obj_set_style_bg_color(get_container(), COLOR_YELLOW, LV_PART_MAIN | LV_STATE_FOCUSED);

        lv_obj_t * type_icon = lv_image_create(get_container());
        lv_obj_align(type_icon, LV_ALIGN_LEFT_MID, 0, 0);
        lv_obj_set_width(type_icon, LV_SIZE_CONTENT);
        lv_obj_set_height(type_icon, LV_SIZE_CONTENT);
        lv_image_set_scale(type_icon, 160);

        if (type == BlowVarTypeEnum::LITRES)
            lv_image_set_src(type_icon, &img_water_glass);
        else
            lv_image_set_src(type_icon, &img_sand_watch);

        lv_obj_t * value_presenter = lv_label_create(get_container());
        lv_obj_align(value_presenter, LV_ALIGN_LEFT_MID, 50, 0);
        lv_obj_set_style_text_font(value_presenter, &OpenSans_bold_20px, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text(value_presenter, "5.000 л.");

        remember_child_element("[label_value]", value_presenter);
        update_ui_context();
    }

    void set_value(int32_t value, string post_fix = "")
    {
        static char buffer[50];

        if (var_type == BlowVarTypeEnum::LITRES)
        {
            sprintf(buffer, "%.3f ", (double)value / 1000.0);
            lv_label_set_text(get_container_content("[label_value]"), (string(buffer) + post_fix).c_str());
        }
        else
        {
            sprintf(buffer, "%02d:%02d", (uint32_t)value / 60, (uint32_t)value % 60);
            lv_label_set_text(get_container_content("[label_value]"), buffer);
        }
    }
};

#endif