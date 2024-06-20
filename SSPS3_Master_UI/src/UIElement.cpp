#include "../include/UIElement.hpp"

UIElement::UIElement(
    vector<EquipmentType> relates_to,
    vector<KeyModel> key_press_actions,
    bool _is_focusable,
    bool _is_container,
    PlaceControlIn bind_to,
    lv_obj_t * lv_screen,
    UIElement * parent_navi
)
{
    this->relates_to = relates_to;
    this->key_press_actions = key_press_actions;
    this->_is_focusable = _is_focusable;
    this->_is_container = _is_container;
    this->lv_screen = lv_screen;
    this->parent_navi = parent_navi;
    
    switch (bind_to)
    {
        case PlaceControlIn::Control: {
            if (this->parent_navi != nullptr)
            {
                context = lv_obj_create(parent_navi->get_container());
                parent_navi->navi_childs.push_back(this);
            }
            else
                context = lv_obj_create(this->lv_screen);
        }; break;

        default: {
            context = lv_obj_create(this->lv_screen);
        }; break;
    }
}

bool UIElement::is_focusable()
{
    return this->_is_focusable;
}

bool UIElement::is_container()
{
    return this->_is_container;
}

bool UIElement::key_press(uint8_t key)
{
    if (key < static_cast<uint8_t>(KeyMap::_END))
        return this->key_press(static_cast<KeyMap>(key));
    return false;
}

bool UIElement::key_press(KeyMap key)
{
    for (auto mapping : key_press_actions)
        if (mapping.trigger(key))
            return true;
    return false;
}

lv_obj_t * UIElement::get_screen()
{
    return this->lv_screen;
}

lv_obj_t * UIElement::get_container()
{
    return this->context;
}

lv_obj_t * UIElement::get_context_child(string key)
{
    if (context_childs.find(key) != context_childs.end())
        return context_childs.find(key)->second;
    return this->context;
}

UIElement * UIElement::add_context_child(string key, lv_obj_t * child)
{
    context_childs.insert({key, child});
    return this;
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

UIElement * UIElement::lv_clear_states()
{
    lv_obj_set_state(context, LV_STATE_DEFAULT, true);
    lv_obj_set_state(context, LV_STATE_FOCUSED, false);
    lv_obj_set_state(context, LV_STATE_PRESSED, false);
    lv_obj_set_state(context, LV_STATE_USER_1, false);
    lv_obj_set_state(context, LV_STATE_USER_2, false);
    return this;
}

UIElement * UIElement::lv_set_focused(bool state)
{
    lv_obj_set_state(context, LV_STATE_FOCUSED, state);
    return this;
}

UIElement * UIElement::lv_set_selected(bool state)
{
    lv_obj_set_state(context, LV_STATE_PRESSED, state);
    return this;
}

UIElement * UIElement::lv_set_transparent(bool state)
{
    lv_obj_set_state(context, LV_STATE_USER_1, state);
    return this;
}

UIElement * UIElement::lv_set_hidden(bool state)
{
    lv_obj_set_state(context, LV_STATE_USER_2, state);
    return this;
}

UIElement * UIElement::get_parent()
{
    return this->parent_navi;
}

UIElement * UIElement::get_selected()
{
    if (this->selected != nullptr)
        return this->selected->get_selected();
    else
        return this;
}

UIElement * UIElement::hide()
{
}

UIElement * UIElement::show()
{
}
