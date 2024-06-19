#include "../include/UIElement.hpp"

UIElement::UIElement(UIScreen * parent, bool is_focusable, vector<UIAccess> ui_access, vector<KeyModel> keys_action)
{
    this->parent = parent;
    this->_is_focusable = is_focusable;
    this->UI_access = ui_access;
    this->KeysAction = keys_action;

    container = lv_obj_create(this->parent->get_lv_obj());
}

void UIElement::add_update_base_info_action(UIRefreshAction action)
{
    UpdateBaseInfo.clear();
    UpdateBaseInfo.push_back(action);
}

void UIElement::add_update_context_action(UIRefreshAction action)
{
    UpdateContext.clear();
    UpdateContext.push_back(action);
}

bool UIElement::is_focusable() {
    return this->_is_focusable;
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
        action(container);
}

void UIElement::update_context()
{
    for (auto action : UpdateContext)
        action(container);
}

void UIElement::ui_obj_remove_states()
{
    lv_obj_set_state(container, LV_STATE_DEFAULT, false);
    lv_obj_set_state(container, LV_STATE_FOCUSED, false);
    lv_obj_set_state(container, LV_STATE_USER_1, false);
    lv_obj_set_state(container, LV_STATE_USER_2, false);
}

void UIElement::ui_show()
{
    ui_obj_remove_states();
    lv_obj_set_state(this->container, LV_STATE_DEFAULT, true);
}

void UIElement::ui_focus()
{
    ui_obj_remove_states();
    lv_obj_set_state(this->container, LV_STATE_FOCUSED, true);
}

void UIElement::ui_transparent()
{
    ui_obj_remove_states();
    lv_obj_set_state(this->container, LV_STATE_USER_1, true);
}

void UIElement::ui_hide()
{
    ui_obj_remove_states();
    lv_obj_set_state(this->container, LV_STATE_USER_2, true);
} 

lv_obj_t * UIElement::get_lv_obj() {
    return this->container;
}

UIScreen * UIElement::get_parent() {
    return this->parent;
}

vector<KeyModel> * UIElement::get_keys_action() {
    return &this->KeysAction;
}