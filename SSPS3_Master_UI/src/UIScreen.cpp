#include "../include/UIElement.hpp"
#include "../include/UIScreen.hpp"


/*

    ui_DateTimeConntainer = lv_obj_create(screen);
    lv_obj_set_width(ui_DateTimeConntainer, 130);
    lv_obj_set_height(ui_DateTimeConntainer, 50);
    lv_obj_set_x(ui_DateTimeConntainer, 165);
    lv_obj_set_y(ui_DateTimeConntainer, -125);
    lv_obj_set_align(ui_DateTimeConntainer, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_DateTimeConntainer, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_radius(ui_DateTimeConntainer, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_DateTimeConntainer, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_DateTimeConntainer, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_DateTimeConntainer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_DateTimeConntainer, lv_color_hex(0x9B9B9B), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui_DateTimeConntainer, 155, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui_DateTimeConntainer, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui_DateTimeConntainer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_x(ui_DateTimeConntainer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(ui_DateTimeConntainer, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

*/

void UIScreen::save_ui_element(string const key, UIElement * ui_element) {
    childs.emplace(key, *ui_element);
}

UIScreen::UIScreen(lv_obj_t * screen, vector<KeyModel> keys_action)
{
    this->KeysAction = keys_action;

    window = lv_obj_create(screen);
    lv_obj_set_style_bg_opa(window, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_width(window, 480);
    lv_obj_set_height(window, 320);
    lv_obj_set_align(window, LV_ALIGN_CENTER);
    lv_obj_clear_flag(window, LV_OBJ_FLAG_SCROLLABLE);
}

void UIScreen::unfocus()
{
    if (this->focused != nullptr)
    {
        this->focused->ui_show();
        this->focused = nullptr;
    }
}

void UIScreen::focus(string key)
{
    if (childs.find(key) != childs.end())
    {
        this->focused = &childs.find(key)->second;
        this->focused->ui_focus();
    }
};

bool UIScreen::key_press(uint8_t key)
{
    if (key < 16)
        return this->key_press(static_cast<KeyMap>(key));

    return false;
}

bool UIScreen::key_press(KeyMap key)
{
    if (focused != nullptr)
        return focused->key_press(key);
    else
    for (auto x : KeysAction)
        if (x.trigger(key))
            return true;

    return false;
}

lv_obj_t * UIScreen::get_lv_obj() {
    return this->window;
}