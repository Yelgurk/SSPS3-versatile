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
static string container_main_name = "[container_main]";

class UIElement
{
private:
    vector<UIAccess> ui_access;
    vector<KeyModel> key_press_actions;
    vector<UIAction> ui_base_actions;
    vector<UIAction> ui_context_actions;
    lv_obj_t * lv_find_or_nullptr(string key);
    void lv_remove_states(lv_obj_t * child_lv_obj, bool is_reset = false);
    void lv_add_states(lv_obj_t * child_lv_obj, _lv_state_t state);

    string _container_main = container_main_name;

protected:
    UIElement * ui_parent = nullptr;
    lv_obj_t * lv_obj_parent = nullptr;
    lv_obj_t * focused_on = nullptr;
    lv_obj_t * control = nullptr;
    
    std::map<string, UIElement> child_ui_elements;
    std::map<string, lv_obj_t*> child_lv_objects;
    
    bool _is_focusable = false;
    bool _is_window = false;
    string _container_1 = "[container_1]",
           _container_2 = "[container_2]",
           _container_3 = "[container_3]",
           _container_4 = "[container_4]",
           _container_5 = "[container_5]";

public:
    UIElement(
        vector<UIAccess> ui_access,
        vector<KeyModel> key_press_actions,
        bool is_focusable,
        lv_obj_t * lv_obj_parent,
        UIElement * ui_parent = nullptr
    );
    UIElement * add_ui_base_action(UIAction action);
    UIElement * add_ui_context_action(UIAction action);
    UIElement * clear_ui_base_action();
    UIElement * clear_ui_context_action();
    UIElement * update_ui_base();
    UIElement * update_ui_context();
    bool key_press(uint8_t key);
    bool key_press(KeyMap key);
    UIElement * lv_states_reset(string key = container_main_name);
    UIElement * lv_set_default(string key = container_main_name);
    UIElement * lv_set_focused(string key = container_main_name);
    UIElement * lv_set_checked(string key = container_main_name);
    UIElement * lv_set_transparent(string key = container_main_name);
    UIElement * lv_set_hidden(string key = container_main_name);
    UIElement * lv_clear_transparent(string key = container_main_name);
    UIElement * lv_clear_hidden(string key = container_main_name);
    bool is_focusable();
    bool is_window();
    UIElement * get_ui_parent_or_nullptr();
    lv_obj_t * get_lv_obj_parent();
    UIElement * hide();
    UIElement * show();
};

#endif