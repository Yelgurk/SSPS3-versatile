#ifndef UITaskListItem_hpp
#define UITaskListItem_hpp

#include "../UIElement.hpp"

struct UITaskItemData
{
    string      name;
    uint8_t     rotation;
    uint8_t     tempC;
    int16_t     duratTotalSS;

    UITaskItemData(string name, uint8_t rotation, uint8_t tempC, int16_t duratTotalSS)
    : name(name), rotation(rotation), tempC(tempC), duratTotalSS(duratTotalSS) {}

    int16_t get_durat_ss();
    int16_t get_durat_mm();
    int16_t get_durat_hh();
};

class UITaskListItem : public UIElement
{
private:
lv_obj_t * header;
lv_obj_t * lab_1;
lv_obj_t * lab_2;
lv_obj_t * lab_3;
UITaskItemData * node;

public:
    UITaskListItem(UIElement * parent_navi, UITaskItemData * node, vector<KeyModel> key_press_actions)
    : UIElement
    {
        { EquipmentType::All },
        key_press_actions,
        true, false, false,
        PlaceControlIn::Control,
        parent_navi->get_screen(),
        parent_navi
    }
    {
        this->node = node;

        lv_obj_set_width(get_container(), 250);
        lv_obj_set_height(get_container(), 40);
        lv_obj_set_style_bg_color(get_container(), lv_color_hex(0xdddddd), 0);

        header = lv_label_create(get_container());
        lv_obj_align(header, LV_ALIGN_LEFT_MID, 10, 0);
        lv_label_set_text(header, "proverka");

        lab_1 = lv_label_create(get_container());
        lv_obj_align(lab_1, LV_ALIGN_RIGHT_MID, -120, 0);
        lv_label_set_text(lab_1, "x1");

        lab_2 = lv_label_create(get_container());
        lv_obj_align(lab_2, LV_ALIGN_RIGHT_MID, -90, 0);
        lv_label_set_text(lab_2, "x2");

        lab_3 = lv_label_create(get_container());
        lv_obj_align(lab_3, LV_ALIGN_RIGHT_MID, -20, 0);
        lv_label_set_text(lab_3, "x3");

        remember_child_element("[header]", header);
        remember_child_element("[val1]", lab_1);
        remember_child_element("[val2]", lab_2);
        remember_child_element("[val3]", lab_3);

        add_ui_base_action([this](){ lv_label_set_text(this->header, this->node->name.c_str()); });
        add_ui_context_action([this]() {
            lv_label_set_text(this->lab_1, to_string(this->node->rotation).c_str());
            lv_label_set_text(this->lab_2, to_string(this->node->tempC).c_str());
            lv_label_set_text(this->lab_3, to_string(this->node->duratTotalSS).c_str());
        });

        update_ui_base();
        update_ui_context();
    }
};

#endif