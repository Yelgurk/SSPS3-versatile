#include "../include/UIElement.hpp"

UIElement::UIElement(
    vector<UIAccess> ui_access,
    vector<KeyModel> key_press_actions,
    bool is_focusable,
    lv_obj_t * lv_obj_parent,
    UIElement * ui_parent
)
{
    this->ui_access = ui_access;
    this->key_press_actions = key_press_actions;
    this->_is_focusable = is_focusable;
    this->lv_obj_parent = lv_obj_parent;

    if (ui_parent == nullptr)
    {
        _is_window = true;
        _is_focusable = false;
    }
    
    this->ui_parent = ui_parent; 

    control = lv_obj_create(this->lv_obj_parent);
    child_lv_objects.emplace(_container_main, control);
}

UIElement * UIElement::add_ui_base_action(UIAction action)
{
    ui_base_actions.push_back(action);
    return this;
}

UIElement * UIElement::add_ui_context_action(UIAction action)
{
    ui_context_actions.push_back(action);
    return this;
}

UIElement * UIElement::clear_ui_base_action()
{
    ui_base_actions.clear();
    return this;
}

UIElement * UIElement::clear_ui_context_action()
{
    ui_context_actions.clear();
    return this;
}

UIElement * UIElement::update_ui_base()
{
    for (auto action : ui_base_actions)
        action();

    return this;
}

UIElement * UIElement::update_ui_context()
{
    for (auto action : ui_context_actions)
        action();

    return this;
}

bool UIElement::key_press(uint8_t key)
{
    if (key < static_cast<uint8_t>(KeyMap::_END))
        return this->key_press(static_cast<KeyMap>(key));

    return false;
}

bool UIElement::key_press(KeyMap key)
{
    for (auto mapped_key : key_press_actions)
        if (mapped_key.trigger(key))
        {
            update_ui_context();
            return true;
        }

    return false;
}

lv_obj_t * UIElement::lv_find_or_nullptr(string key)
{
    if (child_lv_objects.find(key) != child_lv_objects.end())
        return child_lv_objects.find(key)->second;

    return nullptr;
}

void UIElement::lv_remove_states(lv_obj_t * child_lv_obj, bool is_reset)
{
    if (child_lv_obj != nullptr)
    {
        lv_obj_set_state(child_lv_obj, LV_STATE_DEFAULT, false);
        lv_obj_set_state(child_lv_obj, LV_STATE_FOCUSED, false);
        lv_obj_set_state(child_lv_obj, LV_STATE_PRESSED, false);
        
        if (is_reset)
        {
            lv_obj_set_state(child_lv_obj, LV_STATE_DEFAULT, true);
            lv_obj_set_state(child_lv_obj, LV_STATE_USER_1, false);
            lv_obj_set_state(child_lv_obj, LV_STATE_USER_2, false);
        }
    }
}

void UIElement::lv_add_states(lv_obj_t * child_lv_obj, _lv_state_t state)
{
    if (child_lv_obj != nullptr)
        lv_obj_set_state(child_lv_obj, state, true);
}

UIElement * UIElement::lv_states_reset(string key)
{
    lv_remove_states(lv_find_or_nullptr(key), true);
    return this;
}

UIElement * UIElement::lv_set_default(string key)
{
    lv_remove_states(lv_find_or_nullptr(key));
    lv_add_states(lv_find_or_nullptr(key), LV_STATE_DEFAULT);
    return this;
}

UIElement * UIElement::lv_set_focused(string key)
{
    lv_set_default(key);
    lv_add_states(lv_find_or_nullptr(key), LV_STATE_DEFAULT);
    return this;
}

UIElement * UIElement::lv_set_checked(string key)
{
    lv_set_focused(key);
    lv_add_states(lv_find_or_nullptr(key), LV_STATE_PRESSED);
    return this;
}

UIElement * UIElement::lv_set_transparent(string key)
{
    lv_add_states(lv_find_or_nullptr(key), LV_STATE_USER_1);
    return this;
}

UIElement * UIElement::lv_set_hidden(string key)
{
    lv_add_states(lv_find_or_nullptr(key), LV_STATE_USER_2);
    return this;
}

UIElement * UIElement::lv_clear_transparent(string key)
{
    if (lv_find_or_nullptr(key) != nullptr)
        lv_obj_set_state(lv_find_or_nullptr(key), LV_STATE_USER_1, false);
    return this;
}

UIElement * UIElement::lv_clear_hidden(string key)
{
    if (lv_find_or_nullptr(key) != nullptr)
        lv_obj_set_state(lv_find_or_nullptr(key), LV_STATE_USER_2, false);
    return this;
}

bool UIElement::is_focusable()
{
    return this->_is_focusable;
}

bool UIElement::is_window()
{
    return this->_is_window;
}

UIElement * UIElement::get_ui_parent_or_nullptr()
{
    return this->ui_parent;
}

lv_obj_t * UIElement::get_lv_obj_parent()
{
    return this->lv_obj_parent;
}

UIElement * UIElement::hide()
{
    for (const auto& lv_obj_tuple : child_lv_objects)
        lv_add_states(lv_obj_tuple.second, LV_STATE_USER_2);
    
    return this;
}

UIElement * UIElement::show()
{
    for (const auto& lv_obj_tuple : child_lv_objects)
        lv_obj_set_state(lv_obj_tuple.second, LV_STATE_USER_2, false);
        
    return this;
}