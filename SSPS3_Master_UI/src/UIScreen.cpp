#include "../include/UIScreen.hpp"

bool UIScreen::key_press(uint8_t key)
{
    if (key < 16)
        return this->key_press(static_cast<KeyMap>(key));

    return false;
}

bool UIScreen::key_press(KeyMap key)
{
    for (auto x : KeysAction)
        if (x.trigger(key))
            return true;

    if (focused != nullptr)
        for (auto x : *focused->get_keys_action())
            if (x.trigger(key))
                return true;

    return false;
}
