#ifndef UIProgramSelectorItem_hpp
#define UIProgramSelectorItem_hpp

#include "../UIElement.hpp"

typedef function<void(uint8_t)> ActionWithUint8tArg;

class UIProgramSelectorItem : public UIElement
{
private:
    const uint16_t height_selector = 40;
    const uint16_t width_selector = 460;
    ActionWithUint8tArg try_run_prog;
    uint8_t prog_index;

public:
    UIProgramSelectorItem(
        UIElement * parent_navi,
        string name,
        ActionWithUint8tArg try_run_prog,
        uint8_t prog_index
    ) : try_run_prog(try_run_prog), prog_index(prog_index), UIElement
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
        true
    }
    {
        set_key_press_actions({
            KeyModel(KeyMap::TOP,           [this]() { this->navi_prev(); }),
            KeyModel(KeyMap::BOTTOM,        [this]() { this->navi_next(); }),
            KeyModel(KeyMap::LEFT_BOT_REL,  [=]() { try_run_prog(prog_index); }),
            KeyModel(KeyMap::RIGHT_BOT_REL, [=]() { try_run_prog(prog_index); })
        });

        lv_obj_set_height(get_container(), height_selector);
        lv_obj_set_width(get_container(), width_selector);
        lv_obj_set_style_bg_color(get_container(), COLOR_YELLOW, LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_set_style_pad_all(get_container(), 1, LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_set_style_pad_all(get_container(), 0, LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_set_style_border_width(get_container(), 0, LV_PART_MAIN | LV_STATE_FOCUSED);

        lv_obj_t * label_name = lv_label_create(get_container());
        lv_obj_set_flex_grow(label_name, 1);
        lv_obj_set_width(label_name, LV_SIZE_CONTENT);
        lv_obj_set_height(label_name, LV_SIZE_CONTENT);
        lv_obj_set_style_max_width(label_name, width_selector - 20, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_align(label_name, LV_ALIGN_CENTER, 0, 0);
        lv_label_set_long_mode(label_name, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_set_style_text_font(label_name, &OpenSans_bold_20px, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text(label_name, name.c_str());

        remember_child_element("[settings_name]", label_name);
    }


};

#endif