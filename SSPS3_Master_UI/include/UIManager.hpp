#pragma once
#ifndef UIManager_hpp
#define UIManager_hpp

#include <Arduino.h>
#include <map>
#include "FRAM_DB.hpp"

using namespace std;

enum class ScreenType : uint8_t {
    NONE,
    PROGRAM_SELECTOR,
    TASK_ROADMAP,
    BLOWING_CONTROL,
    MENU_USER,
    MENU_MASTER
};

//#define PASS_V0
#define PASS_V1

#ifdef PASS_V0
#define PASS_BUFFER_SIZE    8
#endif

#ifdef PASS_V1
#define PASS_BUFFER_SIZE    6
#endif

class CircularPassBuffer
{
private:
#ifdef PASS_V0
    const KeyMap key[PASS_BUFFER_SIZE] = {
        KeyMap::L_STACK_4,
        KeyMap::L_STACK_1,
        KeyMap::L_STACK_3,
        KeyMap::L_STACK_2,
        KeyMap::R_STACK_4,
        KeyMap::R_STACK_1,
        KeyMap::R_STACK_3,
        KeyMap::R_STACK_2
    };
#endif

#ifdef PASS_V1
    const KeyMap key[PASS_BUFFER_SIZE] = {
        KeyMap::L_STACK_4,
        KeyMap::L_STACK_4,
        KeyMap::L_STACK_4,
        KeyMap::R_STACK_4,
        KeyMap::R_STACK_4,
        KeyMap::R_STACK_4
    };
#endif

    KeyMap buffer[PASS_BUFFER_SIZE] = { KeyMap::_END };

public:
    void add(uint16_t key)
    {
        if (key < static_cast<uint16_t>(KeyMap::_END) / 2)
            add(static_cast<KeyMap>(key));
    }

    void add(KeyMap value)
    {
        for (int i = 0; i < PASS_BUFFER_SIZE - 1; i++)
            buffer[i] = buffer[i + 1];
        buffer[PASS_BUFFER_SIZE - 1] = value;
    }

    bool is_pass_match()
    {
        for (int i = 0; i < PASS_BUFFER_SIZE; i++)
            if (key[i] != buffer[i])
                return false;

        add(KeyMap::_END);
        return true;
    }
};

class UIManager
{
private:
    UIElement* current_control;
    ScreenType current_control_type;
    std::map<ScreenType, UIElement*> screens;
    CircularPassBuffer _master_settings_pass = CircularPassBuffer();

public:
    UIManager() :
    current_control(nullptr),
    current_control_type(ScreenType::NONE)
    {}

    void add_control(ScreenType screen_type, UIElement* control) {
        screens[screen_type] = control;
    }

    void set_control(ScreenType screen_type)
    {
        EquipmentType eq_type = var_type_of_equipment_enum.get();
        bool is_chm     = eq_type == EquipmentType::Cheesemaker;
        bool is_pasteur =
            eq_type == EquipmentType::Pasteurizer ||
            eq_type == EquipmentType::DairyTaxiPasteurizer ||
            eq_type == EquipmentType::DairyTaxiPasteurizerFlowgun;
        bool is_blowgun =
            eq_type == EquipmentType::DairyTaxiFlowgun ||
            eq_type == EquipmentType::DairyTaxiPasteurizerFlowgun;
        
        if (screen_type == ScreenType::TASK_ROADMAP && !is_pasteur && !is_chm)
            screen_type = ScreenType::MENU_USER;
        
        if (screen_type == ScreenType::BLOWING_CONTROL && !is_blowgun)
            screen_type = ScreenType::MENU_USER;

        auto it = screens.find(screen_type);

        if (it != screens.end())
        {
            if (current_control)
                current_control->hide_ui_hierarchy();
            
            current_control = it->second;
            current_control_type = screen_type;
            
            if (current_control)
                current_control->show_ui_hierarchy();
        }
    }

    bool handle_key_press(uint8_t key)
    {
        if (current_control_type == ScreenType::NONE)
            return false;

        switch (current_control_type)
        {
        case ScreenType::PROGRAM_SELECTOR: {
            current_control->get_selected()->key_press(key);
            current_control->get_selected(true)->key_press(key);
        }; break;
        
        case ScreenType::TASK_ROADMAP: {
            current_control->get_selected()->key_press(key);
            current_control->get_selected(true)->key_press(key);
        }; break;
        
        case ScreenType::BLOWING_CONTROL: {
            current_control->get_selected()->key_press(key);
            current_control->get_selected(true)->key_press(key);
        }; break;
        
        case ScreenType::MENU_USER: {
            if (current_control->is_selected_on_child())
                current_control->get_selected(true)->key_press(key);
            current_control->get_selected()->key_press(key);
        }; break;
        
        case ScreenType::MENU_MASTER: {
            if (current_control->is_selected_on_child())
                current_control->get_selected(true)->key_press(key);
            current_control->get_selected()->key_press(key);
        }; break;
        
        default:
            break;
        }

        if (key != static_cast<uint8_t>(KeyMap::_END) && is_current_control(ScreenType::MENU_USER))
        {
            _master_settings_pass.add(key);
            if (_master_settings_pass.is_pass_match())
            {
                screens[ScreenType::MENU_USER]->key_press(KeyMap::LEFT_TOP);
                screens[ScreenType::MENU_USER]->key_press(KeyMap::LEFT_TOP);
                set_control(ScreenType::MENU_MASTER);
            }
        }

        return true;
    }

    bool is_current_control(ScreenType screen_type) {
        return current_control_type == screen_type;
    }
};

#endif