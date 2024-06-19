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

class UIScreen;
typedef function<void()> UIRefreshAction;

class UIElement
{
private:
    vector<UIAccess> UI_access;
    vector<KeyModel> KeysAction;
    vector<UIRefreshAction> UpdateBaseInfo;
    vector<UIRefreshAction> UpdateContext;

    UIScreen * parent = nullptr;
    bool _is_focusable = false;
    
    void ui_obj_remove_states();

protected:
    lv_obj_t * container = nullptr;
    std::map<string, lv_obj_t*> childs;

public:
    UIElement(UIScreen * parent, bool is_focusable, vector<UIAccess> ui_access, vector<KeyModel> keys_action);
    void add_update_base_info_action(UIRefreshAction action);
    void add_update_context_action(UIRefreshAction action);
    bool is_focusable();
    bool key_press(uint8_t key);
    bool key_press(KeyMap key);
    void update_base_info();
    void update_context();
    void ui_show();
    void ui_focus();
    void ui_transparent();
    void ui_hide();
    lv_obj_t * get_lv_obj();
    UIScreen * get_parent();
    vector<KeyModel> * get_keys_action();
    std::map<string, lv_obj_t*> * get_childs();
};

#endif