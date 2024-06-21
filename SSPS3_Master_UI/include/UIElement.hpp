#ifndef UIElement_hpp
#define UIElement_hpp

#include <Arduino.h>
#include <vector>
#include <iostream>
#include <map>
#include <lvgl.h>
#include "UIAccess.hpp"
#include "KeyModel.hpp"

using namespace std;


typedef function<void()> UIAction;

class UIElement
{
private:
    vector<EquipmentType> relates_to;
    vector<KeyModel> key_press_actions;
    vector<UIAction> ui_base_actions;
    vector<UIAction> ui_context_actions;
    vector<UIElement*> navi_childs;

    std::map<string, lv_obj_t*> container_content;

    lv_obj_t * lv_screen = nullptr;
    lv_obj_t * container = nullptr;
    lv_obj_t * navi_childs_presenter = nullptr;

    UIElement * parent_navi = nullptr;
    UIElement * navi_pointer = nullptr;
    UIElement * selected = nullptr;

    bool _is_focusable = false;
    bool _is_container = false;

    void clear_navi_styles();

protected:
    lv_obj_t * get_screen();
    lv_obj_t * get_container();
    lv_obj_t * get_navi_childs_presenter();
    
    UIElement * set_child_presenter(string key);
    UIElement * remember_container_child(string key, lv_obj_t * child);

public:
    UIElement(
        vector<EquipmentType> relates_to,
        vector<KeyModel> key_press_actions,
        bool _is_focusable,
        bool _is_container,
        PlaceControlIn bind_to,
        lv_obj_t * lv_screen,
        UIElement * parent_navi = nullptr
    );
    lv_obj_t * get_container_content(string key);
    bool is_focusable();
    bool is_container();
    bool key_press(uint8_t key);
    bool key_press(KeyMap key);
    UIElement * navi_next();
    UIElement * navi_prev();
    UIElement * navi_ok();
    UIElement * navi_back();
    UIElement * add_ui_base_action(UIAction action);
    UIElement * add_ui_context_action(UIAction action);
    UIElement * clear_ui_base_action();
    UIElement * clear_ui_context_action();
    UIElement * update_ui_base();
    UIElement * update_ui_context();
    UIElement * lv_clear_states();
    UIElement * lv_set_focused(bool state);
    UIElement * lv_set_selected(bool state);
    UIElement * lv_set_transparent(bool state);
    UIElement * lv_set_hidden(bool state);
    UIElement * get_parent();
    UIElement * get_selected();
    UIElement * hide();
    UIElement * show();
};

#endif