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
                container = lv_obj_create(parent_navi->get_navi_childs_presenter());
            else
                container = lv_obj_create(this->lv_screen);
        }; break;

        default: {
            container = lv_obj_create(this->lv_screen);
        }; break;
    }
    
    if (this->parent_navi != nullptr)
        parent_navi->navi_childs.push_back(this);

    this->navi_childs_presenter = this->container;

    lv_obj_set_style_pad_all(container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(container, 2, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_border_color(container, lv_obj_get_style_bg_color(container, LV_PART_MAIN | LV_STATE_DEFAULT), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(container, lv_color_hex(0xFFD800), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_border_color(container, lv_color_hex(0xFFD800), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(container, lv_color_hex(0xFFD800), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_opa(container, 120, LV_PART_MAIN | LV_STATE_USER_1);
    lv_obj_set_style_opa(container, 0, LV_PART_MAIN | LV_STATE_USER_2);
}

void UIElement::clear_navi_styles()
{
    for (const auto& n_c : navi_childs)
        n_c->lv_clear_states();
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

UIElement * UIElement::navi_next()
{
    if (navi_pointer == nullptr)
    {
        if (navi_childs.size() > 0)
            navi_pointer = navi_childs.front();
    }
    else
    {
        navi_pointer->lv_clear_states();

        auto it = std::find(navi_childs.begin(), navi_childs.end(), navi_pointer);
        if(it != navi_childs.end())
        {
            if(std::next(it) != navi_childs.end())
                navi_pointer = *(std::next(it));
            else
                navi_pointer = navi_childs.front();
        }
    }

    if (navi_pointer != nullptr)
        navi_pointer->lv_set_focused(true);

    return this;
}

UIElement * UIElement::navi_prev()
{
    if (navi_pointer == nullptr)
    {
        if (navi_childs.size() > 0)
            navi_pointer = navi_childs.front();
    }
    else
    {
        navi_pointer->lv_clear_states();

        auto it = std::find(navi_childs.begin(), navi_childs.end(), navi_pointer);

        if (it != navi_childs.begin())
            navi_pointer = *(--it);
        else
            navi_pointer = navi_childs.back();
    }

    if (navi_pointer != nullptr)
        navi_pointer->lv_set_focused(true);
    
    return this;
}

UIElement * UIElement::navi_ok()
{
    if (navi_pointer != nullptr)
    {
        selected = navi_pointer;
        selected
        ->lv_clear_states()
        ->lv_set_selected(true);
    }
    return this;
}

UIElement * UIElement::navi_back()
{
    clear_navi_styles();
    lv_clear_states();
    navi_pointer = selected = nullptr;

    if (parent_navi != nullptr)
    {
        parent_navi->selected = nullptr;
        parent_navi->navi_pointer = this;
    }

    if (!_is_container)
        lv_set_focused(true);
    return this;
}

lv_obj_t * UIElement::get_screen()
{
    return this->lv_screen;
}

lv_obj_t * UIElement::get_container()
{
    return this->container;
}

lv_obj_t * UIElement::get_container_content(string key)
{
    if (container_content.find(key) == container_content.end())
        return this->container;
    else
        return container_content.find(key)->second;
}

lv_obj_t * UIElement::get_navi_childs_presenter()
{
    return this->navi_childs_presenter;
}

UIElement * UIElement::set_child_presenter(string key)
{
    this->navi_childs_presenter = this->get_container_content(key);
    return this;
}

UIElement * UIElement::remember_container_child(string key, lv_obj_t * child)
{
    container_content.insert({key, child});
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
    lv_obj_set_state(container, LV_STATE_DEFAULT, true);
    lv_obj_set_state(container, LV_STATE_FOCUSED, false);
    lv_obj_set_state(container, LV_STATE_PRESSED, false);
    lv_obj_set_state(container, LV_STATE_USER_1, false);
    lv_obj_set_state(container, LV_STATE_USER_2, false);
    return this;
}

UIElement * UIElement::lv_set_focused(bool state)
{
    lv_obj_set_state(container, LV_STATE_FOCUSED, state);
    return this;
}

UIElement * UIElement::lv_set_selected(bool state)
{
    lv_obj_set_state(container, LV_STATE_PRESSED, state);
    return this;
}

UIElement * UIElement::lv_set_transparent(bool state)
{
    lv_obj_set_state(container, LV_STATE_USER_1, state);
    return this;
}

UIElement * UIElement::lv_set_hidden(bool state)
{
    lv_obj_set_state(container, LV_STATE_USER_2, state);
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
