#include "../include/KeyModel.hpp"

KeyModel::KeyModel(KeyMap key, KeyAction OnClick)
{
    this->key = key;
    key_action.push_back(OnClick);
}

bool KeyModel::trigger(uint8_t triggered_key)
{
    if (triggered_key < 16)
        return this->trigger(static_cast<KeyMap>(triggered_key));
    
    return false;
}

bool KeyModel::trigger(KeyMap triggered_key)
{
    if (triggered_key == this->key)
    {
        key_action.front()();
        return true;
    }

    return false;
}