#ifndef UIScreen_hpp
#define UIScreen_hpp

#include <Arduino.h>
#include <vector>
#include <iostream>
#include <map>
#include <lvgl.h>
#include "UIAccess.hpp"
#include "KeyModel.hpp"

class UIElement;
using namespace std;

class UIScreen
{
protected:
    vector<UIAccess> UI_access;
    vector<KeyModel> KeysAction;
    std::map<string, UIElement> childs;
    
    UIElement * focused = nullptr;
    lv_obj_t * window;

public:
    UIScreen(lv_obj_t * screen, vector<KeyModel> keys_action);
    void save_ui_element(string const key, UIElement * ui_element);
    void unfocus();
    void focus(string key);
    bool key_press(uint8_t key);
    bool key_press(KeyMap key);
    lv_obj_t * get_lv_obj();
};

#endif