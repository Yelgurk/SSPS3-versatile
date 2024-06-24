#ifndef UIClock1_hpp
#define UIClock1_hpp

#include "../UIElement.hpp"
#include "./list_item.hpp"

class UIClock1 : public UIElement
{
private:
    vector<UILItem*> _collection;

public:
    UIClock1(
        vector<EquipmentType> relates_to,
        vector<KeyModel> key_press_actions,
        bool _is_focusable,
        bool _is_selectable,
        bool _is_container,
        PlaceControlIn bind_to,
        lv_obj_t * lv_screen,
        UIElement * parent_navi = nullptr)
        : UIElement{relates_to, key_press_actions, _is_focusable, _is_selectable, _is_container, bind_to, lv_screen, parent_navi}
    {
        lv_obj_align(get_container(), LV_ALIGN_TOP_LEFT, 100, 0);
        lv_obj_set_width(get_container(), 300);
        lv_obj_set_height(get_container(), 240);

        lv_obj_t * listik = lv_list_create(get_container());
        lv_obj_set_style_radius(listik, 0, 0);
        lv_obj_set_style_border_width(listik, 0, 0);
        lv_obj_align(listik, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_set_width(listik, 300);
        lv_obj_set_height(listik, 240);
        lv_obj_set_style_bg_color(listik, lv_color_hex(0xbbbbbb), 0);

        this->remember_child_element("[list]", listik);
        this->set_childs_presenter("[list]");
    }

    void clear_list()
    {
        lv_obj_clean(this->get_navi_childs_presenter());

        for (auto item : _collection)
        {
            item->delete_ui_element(false);
            delete item;   
        }
        _collection.clear();
    }

    void load_list(vector<TaskData> * list)
    {
        clear_list();
        for (uint16_t i = 0; i < list->size(); i++)
            _collection.push_back(new UILItem(this, &list->at(i), {
                KeyModel(KeyMap::R_STACK_4, []() { Serial.println("Нажата фокус из списка"); })
            }));
    }
};

#endif