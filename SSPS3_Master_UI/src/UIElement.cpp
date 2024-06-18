#include "../include/UIElement.hpp"

static void lv_obj_remove_states(lv_obj_t* lv_obj)
{
    lv_obj_set_state(lv_obj, LV_STATE_DEFAULT, false);
    lv_obj_set_state(lv_obj, LV_STATE_FOCUSED, false);
    lv_obj_set_state(lv_obj, LV_STATE_USER_1, false);
    lv_obj_set_state(lv_obj, LV_STATE_USER_2, false);
}

static void lv_obj_show(lv_obj_t* lv_obj)
{
    lv_obj_remove_states(lv_obj);
    lv_obj_set_state(lv_obj, LV_STATE_DEFAULT, true);
}

static void lv_obj_focus(lv_obj_t* lv_obj)
{
    lv_obj_remove_states(lv_obj);
    lv_obj_set_state(lv_obj, LV_STATE_FOCUSED, true);
}

static void lv_obj_transparent(lv_obj_t* lv_obj)
{
    lv_obj_remove_states(lv_obj);
    lv_obj_set_state(lv_obj, LV_STATE_USER_1, true);
}

static void lv_obj_hide(lv_obj_t* lv_obj)
{
    lv_obj_remove_states(lv_obj);
    lv_obj_set_state(lv_obj, LV_STATE_USER_2, true);
} 

bool UIElement::key_press(uint8_t key)
{
    if (key < 16)
        return this->key_press(static_cast<KeyMap>(key));

    return false;
}

bool UIElement::key_press(KeyMap key)
{
    for (auto x : KeysAction)
        if (x.trigger(key))
        {
            update_context();
            return true;
        }

    return false;
}

void UIElement::update_base_info()
{
    for (auto action : UpdateBaseInfo)
        action();
}

void UIElement::update_context()
{
    for (auto action : UpdateContext)
        action();
}

lv_obj_t * UIElement::get_lv_obj() {
    return this->container;
}

vector<KeyModel> * UIElement::get_keys_action() {
    return &this->KeysAction;
}