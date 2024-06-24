#include "../include/UIElement.hpp"

UIElement::UIElement(
    vector<EquipmentType> relates_to,
    vector<KeyModel> key_press_actions,
    bool is_focusable,
    bool is_selectable,
    bool is_container,
    PlaceControlIn bind_to,
    lv_obj_t * lv_screen,
    UIElement * parent_navi
)
{
    this->relates_to = relates_to;
    this->key_press_actions = key_press_actions;
    this->_is_focusable = is_focusable;
    this->_is_selectable = is_selectable;
    this->_is_container = is_container;
    this->lv_screen = lv_screen;
    this->parent_navi = parent_navi;

    _is_focusable = _is_container ? false : _is_focusable;

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

    this
    ->gui_container_set_rect_design()
    ->gui_container_set_unscrollable()
    ->gui_container_set_focus_style()
    ->gui_container_set_select_style()
    ->gui_container_set_transp_and_hide_style();
}

UIElement * UIElement::clear_navi_styles()
{
    for (const auto& n_c : navi_childs)
        n_c->lv_clear_states();
    return this;
}

UIElement * UIElement::gui_container_set_rect_design()
{
    lv_obj_set_style_pad_all(container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    return this;
}

UIElement * UIElement::gui_container_set_unscrollable()
{
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    return this;
}

UIElement * UIElement::gui_container_set_focus_style()
{
    if (_is_focusable)
    {
        lv_obj_set_style_pad_all(container, -2, LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_set_style_border_width(container, 2, LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_set_style_border_color(container, lv_color_hex(0xFFD800), LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_set_style_border_color(container, lv_color_hex(0xFFD800), LV_PART_MAIN | LV_STATE_PRESSED);
    }
    return this;
}

UIElement * UIElement::gui_container_set_select_style()
{
    if (_is_focusable)
    {
        lv_obj_set_style_bg_color(container, lv_color_hex(0xFFD800), LV_PART_MAIN | LV_STATE_PRESSED);
    }
    return this;
}

UIElement * UIElement::gui_container_set_transp_and_hide_style()
{
    lv_obj_set_style_opa(container, 120, LV_PART_MAIN | LV_STATE_USER_1);
    lv_obj_set_style_opa(container, 0, LV_PART_MAIN | LV_STATE_USER_2);
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
    for (auto mapping : key_press_actions)
        if (mapping.trigger(key))
            return true;
    return false;
}

UIElement * UIElement::navi_next()
{
    if (navi_childs.size() > 0)
    {
        if (navi_pointer == nullptr)
            navi_pointer = navi_childs.back();

        navi_pointer->lv_clear_states();

        auto it = std::find(
            navi_childs.begin(),
            navi_childs.end(),
            navi_pointer
        );

        if (it != navi_childs.end())
        {
            ++it;
            while (it != navi_childs.end())
            {
                if ((*it)->_is_focusable)
                {
                    navi_pointer = *it;
                    break;
                }
                ++it;
            }
        }

        if (it == navi_childs.end())
        {
            it = std::find_if(
                navi_childs.begin(),
                navi_childs.end(),
                [](UIElement* elem) { return elem->_is_focusable; }
            );
            if (it != navi_childs.end())
                navi_pointer = *it;
            else
                navi_pointer = nullptr;
        }
    }
    else
        navi_pointer = nullptr;

    if (navi_pointer != nullptr)
        navi_pointer->lv_set_focused(true);

    return this;
}

UIElement * UIElement::navi_prev()
{
    if (navi_childs.size() > 0)
    {
        if (navi_pointer == nullptr)
            navi_pointer = navi_childs.front();

        navi_pointer->lv_clear_states();

        auto it = std::find(
            navi_childs.rbegin(),
            navi_childs.rend(),
            navi_pointer
        );

        if (it != navi_childs.rend())
        {
            ++it;
            while (it != navi_childs.rend())
            {
                if ((*it)->_is_focusable)
                {
                    navi_pointer = *it;
                    break;
                }
                ++it;
            }
        }

        if (it == navi_childs.rend())
        {
            it = std::find_if(
                navi_childs.rbegin(),
                navi_childs.rend(),
                [](UIElement* elem) { return elem->_is_focusable; }
            );
            if (it != navi_childs.rend())
                navi_pointer = *it;
            else
                navi_pointer = nullptr;
        }
    }
    else
        navi_pointer = nullptr;

    if (navi_pointer != nullptr)
        navi_pointer->lv_set_focused(true);
    
    return this;
}

UIElement * UIElement::navi_ok()
{
    if (navi_pointer != nullptr && navi_pointer->_is_selectable)
    {
        selected = navi_pointer;
        selected
        ->lv_clear_states()
        ->lv_set_selected(true)
        ->navi_next();
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

UIElement * UIElement::set_childs_presenter(string key)
{
    this->navi_childs_presenter = this->get_container_content(key);
    return this;
}

UIElement * UIElement::remember_child_element(string key, lv_obj_t * child)
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

UIElement * UIElement::get_selected(bool get_focused)
{
    if (this->selected != nullptr)
        return this->selected->get_selected(get_focused);
    else if (get_focused && this->navi_pointer != nullptr)
        return this->navi_pointer;
    else
        return this;
}

int16_t UIElement::get_focused_index() 
{ 
    auto it = find(navi_childs.begin(), navi_childs.end(), navi_pointer); 
  
    if (it != navi_childs.end())  
        return it - navi_childs.begin();
    else
        return -1;
} 

UIElement * UIElement::hide()
{
    return this;
}

UIElement * UIElement::show()
{
    return this;
}

void UIElement::clear_ui_childs()
{
    for (auto child : navi_childs)
        child->clear_ui_childs();
    navi_childs.clear();

    if (container != navi_childs_presenter)
        lv_obj_clean(navi_childs_presenter);
    lv_obj_clean(container);
}

void UIElement::delete_ui_element(bool is_dynamic_alloc)
{
    clear_ui_childs();

    if (container != navi_childs_presenter)
        lv_obj_delete(navi_childs_presenter);
    lv_obj_delete(container);

    if (is_dynamic_alloc)
        delete this;
}