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

#define FOCUS_BORDER_WIDTH_PX   4

#define COLOR_WHITE             lv_color_hex(0xFFFFFF)
#define COLOR_WHITE_SMOKE       lv_color_hex(0xF5F5F5)
#define COLOR_GREY              lv_color_hex(0xB5B8B1)
#define COLOR_DARK_GREY         lv_color_hex(0x2F353B)     
#define COLOR_YELLOW            lv_color_hex(0xFFDC33)
#define COLOR_RED               lv_color_hex(0xE34234)
#define COLOR_GREEN             lv_color_hex(0xBDECB6)
#define COLOR_BLUE              lv_color_hex(0XB0E0E6)

typedef function<void()> UIAction;

static const vector<StyleActivator> default_activator = {
    StyleActivator::Rectangle,
    StyleActivator::Unscrollable,
    StyleActivator::Focus,
    StyleActivator::Select
};

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
    bool _is_selectable = false;
    bool _is_container = false;

    bool exists_in_the_collection(vector<StyleActivator> * activator, StyleActivator found);

    UIElement * clear_navi_states();
    UIElement * gui_set_default_style(lv_obj_t * lv_obj);
    UIElement * gui_set_rect_style(lv_obj_t * lv_obj);
    UIElement * gui_set_unscrollable(lv_obj_t * lv_obj);
    UIElement * gui_set_shadow_style(lv_obj_t * lv_obj);
    UIElement * gui_set_focus_style(lv_obj_t * lv_obj);
    UIElement * gui_set_select_style(lv_obj_t * lv_obj);
    UIElement * gui_set_transp_and_hide_style(lv_obj_t * lv_obj);

protected:
    lv_obj_t * get_container();
    lv_obj_t * get_navi_childs_presenter();
    
    UIElement * set_childs_presenter(string key);
    UIElement * remember_child_element(string key, lv_obj_t * child);

public:
    UIElement(
        vector<EquipmentType> relates_to,
        vector<KeyModel> key_press_actions,
        bool is_focusable,
        bool is_selectable,
        bool is_container,
        PlaceControlIn bind_to,
        lv_obj_t * lv_screen,
        UIElement * parent_navi = nullptr,
        vector<StyleActivator> styles_activator = default_activator
    );
    lv_obj_t * get_container_content(string key);
    lv_obj_t * get_screen();
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
    UIElement * get_selected(bool get_focused = false);
    int16_t get_focused_index();
    UIElement * hide();
    UIElement * show();
    void clear_ui_childs();
    void delete_ui_element(bool is_dynamic_alloc);
};

#endif