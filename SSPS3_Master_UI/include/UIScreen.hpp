#pragma once

#ifndef UIScreen_hpp
#define UIScreen_hpp

#include <Arduino.h>
#include <vector>
#include <lvgl.h>
#include "UIAccess.hpp"
#include "UIElement.hpp"

class UIScreen
{
protected:
    vector<UIAccess> UI_access;
    vector<UIElement> childs;
    vector<KeyModel> KeysAction;
    
    UIElement * focused = nullptr;
    lv_obj_t * window;

public:
    bool key_press(uint8_t key);
    bool key_press(KeyMap key);
    lv_obj_t * get_lv_obj();

    /*
    добавить конструктор инициализирующий window и
    что бы все базовые от него классы сперва вызывали базовый конструктор 
    */
};

#endif