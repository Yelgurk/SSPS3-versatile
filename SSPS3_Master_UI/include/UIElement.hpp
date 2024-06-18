#pragma once

#ifndef UIElement_hpp
#define UIElement_hpp

#include <Arduino.h>
#include <vector>
#include <lvgl.h>
#include "UIAccess.hpp"
#include "UIScreen.hpp"
#include "KeyModel.hpp"

using namespace std;

#define LV_OBJ_SHOW(lv_obj)     lv_obj_show(lv_obj)  
#define LV_OBJ_FOCUS(lv_obj)    lv_obj_focus(lv_obj)
#define LV_OBJ_TRANSP(lv_obj)   lv_obj_transparent(lv_obj)
#define LV_OBJ_HIDE(lv_obj)     lv_obj_hide(lv_obj)     

static void lv_obj_remove_states(lv_obj_t* lv_obj);
static void lv_obj_show(lv_obj_t* lv_obj);
static void lv_obj_focus(lv_obj_t* lv_obj);
static void lv_obj_transparent(lv_obj_t* lv_obj);
static void lv_obj_hide(lv_obj_t* lv_obj);

typedef function<void()> UIRefreshAction;
typedef function<void(lv_obj_t*)> UIRefreshActionArg;

class UIElement
{
protected:
    vector<UIAccess> UI_access;
    vector<KeyModel> KeysAction;
    vector<UIRefreshAction> UpdateBaseInfo;
    vector<UIRefreshAction> UpdateContext;

    UIScreen * parent = nullptr;
    lv_obj_t * container = nullptr;

public:
    bool is_focusable();
    bool key_press(uint8_t key);
    bool key_press(KeyMap key);
    void update_base_info();
    void update_context();
    lv_obj_t * get_lv_obj();
    vector<KeyModel> * get_keys_action();

    
    /*
    добавить конструктор:
    - принимающий своего parent
    - инициализирующий container
    - добавляющий container в parent
    и что бы все базовые от него классы сперва вызывали базовый конструктор
    */
};

#endif

/*
Базовый класс для элементов экрана.
Содержит:\
- делегат отвечающий за обновление инфы в контроле
- все методы по установке вышеперечисленного

Предполагается создавать ui-элементы наследуясь от этого класса,
а затем все классы ui-элементов сделать наследуемыми для их общего ui-screen контейнера
*/
