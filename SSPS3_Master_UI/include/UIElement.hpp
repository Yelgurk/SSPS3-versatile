#ifndef UIElement_hpp
#define UIElement_hpp

#include "../resource/lv_resources.hpp"

#include <Arduino.h>
#include <vector>
#include <iostream>
#include <map>
#include <lvgl.h>
#include "UIAccess.hpp"
#include "KeyModel.hpp"
#include "LangDictionary.hpp"

using namespace std;

#define FOCUS_BORDER_WIDTH_PX   4

#define COLOR_WHITE             lv_color_hex(0xFFFFFF)
#define COLOR_WHITE_SMOKE       lv_color_hex(0xF5F5F5)

#define COLOR_GREY              lv_color_hex(0xC8C8C8)
#define COLOR_MEDIUM_GREY       lv_color_hex(0xB5B8B1)
#define COLOR_DARK_GREY         lv_color_hex(0x2F353B)    

#define COLOR_YELLOW            lv_color_hex(0xFFDC33)
#define COLOR_YELLOW_SMOKE      lv_color_hex(0xFDEAA8)

#define COLOR_RED               lv_color_hex(0xE34234)
#define COLOR_RED_INDIAN        lv_color_hex(0xCF7272)

#define COLOR_PINK              lv_color_hex(0xFDBDBA)

#define COLOR_GREEN             lv_color_hex(0xBDECB6)
#define COLOR_GREEN_MEDIUM      lv_color_hex(0x8CCB5E)
#define COLOR_DARK_GREEN        lv_color_hex(0x8CCB5E)       

#define COLOR_BLUE              lv_color_hex(0x80DAEB)
#define COLOR_SKY_BLUE          lv_color_hex(0XAFEEEE)
#define COLOR_DUST_BLUE         lv_color_hex(0xD7EEEF)

LV_FONT_DECLARE(OpenSans_bold_12px);
LV_FONT_DECLARE(OpenSans_bold_14px);
LV_FONT_DECLARE(OpenSans_bold_16px);
LV_FONT_DECLARE(OpenSans_bold_20px);
LV_FONT_DECLARE(OpenSans_bold_24px);
LV_FONT_DECLARE(OpenSans_regular_12px);
LV_FONT_DECLARE(OpenSans_regular_14px);
LV_FONT_DECLARE(OpenSans_regular_16px);
LV_FONT_DECLARE(OpenSans_regular_20px);
LV_FONT_DECLARE(OpenSans_regular_24px);

typedef function<void()> UIAction;

static const vector<StyleActivator> default_styles_activator = {
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
    bool _is_childs_changes_transparency = false;
    bool _is_focus_extra_style_lvl = false;

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
    int32_t _focus_offset_y = 0;
    lv_obj_t * get_container();
    
    void set_key_press_actions(vector<KeyModel> key_press_actions);
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
        vector<StyleActivator> styles_activator = default_styles_activator,
        bool is_childs_changes_transparency = false,
        int32_t focus_offset_y = 0,
        bool is_focus_extra_style_lvl = false
    );
    lv_obj_t * get_container_content(string key);
    lv_obj_t * get_navi_childs_presenter();
    lv_obj_t * get_screen();
    bool is_container();
    bool is_selected_on_child();
    bool key_press(uint8_t key);
    bool key_press(KeyMap key);
    UIElement * focus_on(uint8_t child_index);
    UIElement * navi_next();
    UIElement * navi_prev();
    UIElement * navi_ok();
    UIElement * navi_back();
    UIElement * add_ui_base_action(UIAction action);
    UIElement * add_ui_context_action(UIAction action, bool call_on_init = true);
    UIElement * clear_ui_base_action();
    UIElement * clear_ui_context_action();
    UIElement * update_ui_base();
    UIElement * update_ui_context();
    UIElement * lv_clear_states(lv_obj_t * lv_obj = nullptr);
    void focus_on_first_child(bool clear_prev_navi_focus_style = true);
    UIElement * set_childs_presenter(string key);
    UIElement * set_childs_presenter(UIElement * presenter);
    UIElement * set_focused(bool state);
    UIElement * set_selected(bool state);
    UIElement * set_transparent(bool state);
    UIElement * set_hidden(bool state);
    UIElement * get_parent();
    UIElement * get_selected(bool get_focused = false);
    UIElement * get_navi_el();
    uint16_t get_childs_count();
    int16_t get_focused_index();
    UIElement * set_childs_hidden();
    UIElement * set_childs_transarent();
    UIElement * set_childs_visible();
    void clear_ui_childs();
    void delete_ui_element(bool is_dynamic_alloc);
    UIElement * hide_ui_hierarchy();
    UIElement * show_ui_hierarchy();
};

#endif