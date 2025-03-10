#include "../include/UIElement.hpp"

UIElement::UIElement(
    vector<EquipmentType> relates_to,
    vector<KeyModel> key_press_actions,
    bool is_focusable,
    bool is_selectable,
    bool is_container,
    PlaceControlIn bind_to,
    lv_obj_t * lv_screen,
    UIElement * parent_navi,
    vector<StyleActivator> styles_activator,
    bool is_childs_changes_transparency,
    int32_t focus_offset_y,
    bool is_focus_extra_style_lvl
)
{
    this->relates_to = relates_to;
    this->key_press_actions = key_press_actions;
    this->_is_focusable = is_focusable;
    this->_is_selectable = is_selectable;
    this->_is_container = is_container;
    this->_is_childs_changes_transparency = is_childs_changes_transparency;
    this->lv_screen = lv_screen;
    this->parent_navi = parent_navi;
    this->_focus_offset_y = focus_offset_y;
    this->_is_focus_extra_style_lvl = is_focus_extra_style_lvl;

    _is_focusable = _is_container ? false : _is_focusable;

    switch (bind_to)
    {
        case PlaceControlIn::Control: {
            if (this->parent_navi != nullptr)
            {
                container = lv_obj_create(parent_navi->get_navi_childs_presenter());
            }
            else
            {
                container = lv_obj_create(this->lv_screen);
            }
        }; break;

        default: {
            container = lv_obj_create(this->lv_screen);
        }; break;
    }

    this->navi_childs_presenter = this->container;

    gui_set_default_style(this->container);
    gui_set_transp_and_hide_style(this->container);
    
#ifndef SSPS3_IS_CHEAP_SOLUTION_YES
    if (exists_in_the_collection(&styles_activator, StyleActivator::Rectangle))
        gui_set_rect_style(this->container);
#else
    gui_set_rect_style(this->container);
#endif

    if (exists_in_the_collection(&styles_activator, StyleActivator::Unscrollable))
        gui_set_unscrollable(this->container);

#ifndef SSPS3_IS_CHEAP_SOLUTION_YES
    if (exists_in_the_collection(&styles_activator, StyleActivator::Shadow))
        gui_set_shadow_style(this->container);
#else
    lv_obj_set_style_shadow_opa(this->container, 0, LV_STATE_DEFAULT);
#endif

    if (exists_in_the_collection(&styles_activator, StyleActivator::Focus))
        gui_set_focus_style(this->container);
    
    if (exists_in_the_collection(&styles_activator, StyleActivator::Select))
        gui_set_select_style(this->container);

    if (this->parent_navi != nullptr)
        parent_navi->navi_childs.push_back(this);
}

bool UIElement::exists_in_the_collection(vector<StyleActivator> * activator, StyleActivator found)
{
    for (auto &key : *activator)
        if (key == found)
            return true;
    
    return false;
}

UIElement * UIElement::clear_navi_states()
{
    for (const auto& n_c : navi_childs)
        n_c->lv_clear_states();
    return this;
}

UIElement * UIElement::gui_set_default_style(lv_obj_t * lv_obj)
{
    lv_obj_set_style_border_width(lv_obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(lv_obj, COLOR_GREY, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(lv_obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(lv_obj, COLOR_WHITE, LV_PART_MAIN | LV_STATE_DEFAULT);
    return this;
}

UIElement * UIElement::gui_set_rect_style(lv_obj_t * lv_obj)
{
    lv_obj_set_style_radius(lv_obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    return this;
}

UIElement * UIElement::gui_set_unscrollable(lv_obj_t * lv_obj)
{
    lv_obj_clear_flag(lv_obj, LV_OBJ_FLAG_SCROLLABLE);
    return this;
}

UIElement * UIElement::gui_set_shadow_style(lv_obj_t * lv_obj)
{
    lv_obj_set_style_shadow_color(lv_obj, lv_color_hex(0x9B9B9B), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(lv_obj, 100, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(lv_obj, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(lv_obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_offset_x(lv_obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_offset_y(lv_obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    return this;
}

UIElement * UIElement::gui_set_focus_style(lv_obj_t * lv_obj)
{
    if (_is_focusable)
    {
        lv_obj_set_style_pad_all(lv_obj, -FOCUS_BORDER_WIDTH_PX, LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_set_style_border_width(lv_obj, FOCUS_BORDER_WIDTH_PX, LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_set_style_border_color(lv_obj, COLOR_BLUE, LV_PART_MAIN | LV_STATE_FOCUSED);

        if (_is_focus_extra_style_lvl)
        {
            lv_obj_set_style_pad_all(lv_obj, -FOCUS_BORDER_WIDTH_PX, LV_PART_MAIN | LV_STATE_USER_4);
            lv_obj_set_style_border_width(lv_obj, FOCUS_BORDER_WIDTH_PX, LV_PART_MAIN | LV_STATE_USER_4);
            lv_obj_set_style_border_color(lv_obj, COLOR_BLUE, LV_PART_MAIN | LV_STATE_USER_4);
        }
    }
    return this;
}

UIElement * UIElement::gui_set_select_style(lv_obj_t * lv_obj)
{
    if (_is_focusable)
    {
        lv_obj_set_style_bg_color(lv_obj, COLOR_YELLOW, LV_PART_MAIN | LV_STATE_PRESSED);
    }
    return this;
}

UIElement * UIElement::gui_set_transp_and_hide_style(lv_obj_t * lv_obj)
{
    lv_obj_set_style_opa(lv_obj, 120, LV_PART_MAIN | LV_STATE_USER_1);
    lv_obj_set_style_opa(lv_obj, 0, LV_PART_MAIN | LV_STATE_USER_2);
    return this;
}

bool UIElement::is_selected_on_child() {
    return selected != nullptr;
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

UIElement* UIElement::focus_on(uint8_t child_index)
{
    if (child_index >= navi_childs.size())
        return this;

    UIElement* old_navi = navi_pointer;

    if (navi_childs.size() > 0)
    {
        if (navi_pointer != nullptr)
            navi_pointer->lv_clear_states();

        navi_pointer = navi_childs[child_index];

        if (navi_pointer->_is_focusable)
        {
            navi_pointer->set_focused(true);

            if (navi_pointer->_focus_offset_y != 0 && old_navi != nullptr && old_navi->_focus_offset_y == 0)
                lv_obj_scroll_to_y(navi_childs_presenter, 0, LV_ANIM_OFF);
            else
                lv_obj_scroll_to_view(navi_pointer->get_container(), LV_ANIM_OFF);
        }
        else
            navi_pointer = nullptr;
    }
    else
        navi_pointer = nullptr;

    return this;
}

UIElement * UIElement::navi_next()
{
    bool first_focus_iteration = false;
    UIElement * old_navi = navi_pointer;

    if (navi_childs.size() > 0)
    {
        if (navi_pointer == nullptr)
        {
            first_focus_iteration = true;
            navi_pointer = navi_childs.back();
        }

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
    {
        navi_pointer->set_focused(true);
        if (navi_pointer->_focus_offset_y != 0 && !first_focus_iteration && old_navi != nullptr && old_navi->_focus_offset_y == 0)
            lv_obj_scroll_to_y(navi_childs_presenter, 0, LV_ANIM_OFF);
        else
            lv_obj_scroll_to_view(navi_pointer->get_container(), LV_ANIM_OFF);
    }

    return this;
}

UIElement * UIElement::navi_prev()
{
    bool first_focus_iteration = false;
    UIElement * old_navi = navi_pointer;

    if (navi_childs.size() > 0)
    {
        if (navi_pointer == nullptr)
        {
            first_focus_iteration = true;
            navi_pointer = navi_childs.front();
        }

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
    {
        navi_pointer->set_focused(true);

        if (navi_pointer->_focus_offset_y != 0 && !first_focus_iteration && old_navi != nullptr && old_navi->_focus_offset_y == 0)
            lv_obj_scroll_to_y(navi_childs_presenter, 0, LV_ANIM_OFF);
        else
            lv_obj_scroll_to_view(navi_pointer->get_container(), LV_ANIM_OFF);
    }
    
    return this;
}

UIElement * UIElement::navi_ok()
{
    if (navi_pointer != nullptr && navi_pointer->_is_selectable)
    {
        selected = navi_pointer;
        selected
        ->lv_clear_states()
        ->set_selected(true)
        ->navi_next();
    }
    return this;
}

UIElement * UIElement::navi_back()
{
    clear_navi_states();
    lv_clear_states();
    navi_pointer = selected = nullptr;

    if (parent_navi != nullptr)
    {
        parent_navi->selected = nullptr;
        parent_navi->navi_pointer = this;
    }

    if (!_is_container)
        set_focused(true);
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

void UIElement::set_key_press_actions(vector<KeyModel> key_press_actions) {
    this->key_press_actions = key_press_actions;
}

UIElement * UIElement::set_childs_presenter(string key)
{
    this->navi_childs_presenter = this->get_container_content(key);
    return this;
}

UIElement * UIElement::set_childs_presenter(UIElement * presenter)
{
    this->navi_childs_presenter = presenter->get_navi_childs_presenter();
    this->navi_childs.clear();
    this->navi_childs = presenter->navi_childs;
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
    update_ui_base();
    return this;
}

UIElement * UIElement::add_ui_context_action(UIAction action, bool call_on_init)
{
    ui_context_actions.push_back(action);

    if (call_on_init)
        update_ui_context();
        
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

UIElement * UIElement::lv_clear_states(lv_obj_t * lv_obj)
{
    if (lv_obj != nullptr)
    {
        lv_obj_set_state(lv_obj, LV_STATE_DEFAULT, true);
        lv_obj_set_state(lv_obj, LV_STATE_FOCUSED, false);
        lv_obj_set_state(lv_obj, LV_STATE_PRESSED, false);
        lv_obj_set_state(lv_obj, LV_STATE_USER_1, false);
        lv_obj_set_state(lv_obj, LV_STATE_USER_2, false);
        lv_obj_set_state(lv_obj, LV_STATE_USER_3, false);
        lv_obj_set_state(lv_obj, LV_STATE_USER_4, false);
    }
    else
    {
        set_childs_hidden();
        lv_obj_set_state(container, LV_STATE_DEFAULT, true);
        lv_obj_set_state(container, LV_STATE_FOCUSED, false);
        lv_obj_set_state(container, LV_STATE_PRESSED, false);
        lv_obj_set_state(container, LV_STATE_USER_1, false);
        lv_obj_set_state(container, LV_STATE_USER_2, false);
        lv_obj_set_state(container, LV_STATE_USER_3, false);
        lv_obj_set_state(container, LV_STATE_USER_4, false);
    }
    return this;
}

void UIElement::focus_on_first_child(bool clear_prev_navi_focus_style)
{
    bool first_focus_iteration = true;

    if (navi_childs.size() > 0)
    {
        if (navi_pointer == nullptr)
            navi_pointer = navi_childs.back();

        if (clear_prev_navi_focus_style && navi_pointer != nullptr)
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
    {
        navi_pointer->set_focused(true);
        if (navi_pointer->_focus_offset_y != 0 && !first_focus_iteration)
        {
            lv_area_t child_pos;
            lv_obj_get_coords(navi_pointer->get_container(), &child_pos);
            lv_obj_scroll_to_y(navi_childs_presenter, child_pos.y1 + _focus_offset_y, LV_ANIM_OFF);
        }
        else
            lv_obj_scroll_to_view(navi_pointer->get_container(), LV_ANIM_OFF);
    }
}

UIElement * UIElement::set_focused(bool state)
{
    lv_obj_set_state(container, LV_STATE_FOCUSED, state);
    if (_is_focus_extra_style_lvl)
        lv_obj_set_state(container, LV_STATE_USER_4, state);
    set_childs_transarent();
    return this;
}

UIElement * UIElement::set_selected(bool state)
{
    lv_obj_set_state(container, LV_STATE_PRESSED, state);
    set_childs_visible();
    return this;
}

UIElement * UIElement::set_transparent(bool state)
{
    if (!_is_focus_extra_style_lvl)
    {
        lv_obj_set_state(container, LV_STATE_USER_1, state);
        set_childs_hidden();
    }
    return this;
}

UIElement * UIElement::set_hidden(bool state)
{
    if (!_is_focus_extra_style_lvl)
    {
        lv_obj_set_state(container, LV_STATE_USER_2, state);
        set_childs_hidden();
    }
    return this;
}

UIElement * UIElement::get_parent()
{
    return this->parent_navi;
}

UIElement * UIElement::get_navi_el() {
    return this->navi_pointer; 
}

uint16_t UIElement::get_childs_count() {
    return navi_childs.size();
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

UIElement * UIElement::set_childs_hidden()
{
    if (_is_childs_changes_transparency && container != navi_childs_presenter)
    {
        lv_obj_set_style_opa(navi_childs_presenter, 0, 0);
    }
    return this;
}

UIElement * UIElement::set_childs_transarent()
{
    if (_is_childs_changes_transparency && container != navi_childs_presenter)
    {
        lv_obj_set_style_bg_color(navi_childs_presenter, COLOR_GREY, 0);
        lv_obj_set_style_opa(navi_childs_presenter, 255, 0);
    }

    for (auto child : navi_childs)
        child->update_ui_context();

    return this;
}

UIElement * UIElement::set_childs_visible()
{
    if (_is_childs_changes_transparency && container != navi_childs_presenter)
    {
        lv_obj_set_style_bg_color(navi_childs_presenter, COLOR_WHITE, 0);
        lv_obj_set_style_opa(navi_childs_presenter, 255, 0);
    }

    return this;
}

void UIElement::clear_ui_childs()
{
    for (auto child : navi_childs)
    {
       child->clear_ui_childs();
       lv_anim_delete(child->container, NULL);
       lv_clear_states();
    }
    
    navi_childs.clear();
    selected = navi_pointer = nullptr;
}

void UIElement::delete_ui_element(bool is_dynamic_alloc)
{
    clear_ui_childs();

    if (container != navi_childs_presenter)
    {
        lv_anim_delete(navi_childs_presenter, NULL);
        lv_obj_delete(navi_childs_presenter);
    }
        
    lv_anim_delete(container, NULL);
    lv_obj_delete(container);

    if (is_dynamic_alloc)
        delete this;
}

UIElement * UIElement::hide_ui_hierarchy()
{
    if (get_container() != get_navi_childs_presenter())
    {
        lv_obj_set_style_opa(get_navi_childs_presenter(), 0, 0);
        lv_obj_add_flag(get_navi_childs_presenter(), LV_OBJ_FLAG_HIDDEN);
    }

    lv_obj_set_style_opa(get_container(), 0, 0);
    lv_obj_add_flag(get_container(), LV_OBJ_FLAG_HIDDEN);
    
    return this;
}

UIElement * UIElement::show_ui_hierarchy()
{
    if (get_container() != get_navi_childs_presenter())
    {
        lv_obj_set_style_opa(get_navi_childs_presenter(), 255, 0);
        lv_obj_remove_flag(get_navi_childs_presenter(), LV_OBJ_FLAG_HIDDEN);
    }

    lv_obj_set_style_opa(get_container(), 255, 0);
    lv_obj_remove_flag(get_container(), LV_OBJ_FLAG_HIDDEN);

    return this;
}