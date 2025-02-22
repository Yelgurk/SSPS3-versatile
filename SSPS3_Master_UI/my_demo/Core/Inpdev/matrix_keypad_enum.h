#pragma once
#ifndef MATRIX_KEYPAD_ENUM_H
#define MATRIX_KEYPAD_ENUM_H

enum KeyState : unsigned char
{
    BTN_CLICK_0, BTN_CLICK_1, BTN_CLICK_2, BTN_CLICK_3,
    BTN_CLICK_4, BTN_CLICK_5, BTN_CLICK_6, BTN_CLICK_7,
    BTN_CLICK_8, BTN_CLICK_9, BTN_CLICK_10, BTN_CLICK_11,
    BTN_CLICK_12, BTN_CLICK_13, BTN_CLICK_14, BTN_CLICK_15,

    BTN_PRESS_0, BTN_PRESS_1, BTN_PRESS_2, BTN_PRESS_3,
    BTN_PRESS_4, BTN_PRESS_5, BTN_PRESS_6, BTN_PRESS_7,
    BTN_PRESS_8, BTN_PRESS_9, BTN_PRESS_10, BTN_PRESS_11,
    BTN_PRESS_12, BTN_PRESS_13, BTN_PRESS_14, BTN_PRESS_15,

    BTN_RELEASE_0, BTN_RELEASE_1, BTN_RELEASE_2, BTN_RELEASE_3,
    BTN_RELEASE_4, BTN_RELEASE_5, BTN_RELEASE_6, BTN_RELEASE_7,
    BTN_RELEASE_8, BTN_RELEASE_9, BTN_RELEASE_10, BTN_RELEASE_11,
    BTN_RELEASE_12, BTN_RELEASE_13, BTN_RELEASE_14, BTN_RELEASE_15,
    
    BTN_DELAY,
    NO_BTN,

    BTN_COUNT           = 16,
    BTN_CLICK_BEGIN     = 0,
    BTN_PRESS_BEGIN     = 16,
    BTN_RELEASE_BEGIN   = 32,

    BTN_CLICK   = 0,
    BTN_PRESS   = 1,
    BTN_RELEASE = 2
};

static KeyState keypad_get_btn_state(KeyState key)
{
    if (key >= BTN_RELEASE_BEGIN + BTN_COUNT)
        return NO_BTN;

    switch (key / BTN_COUNT)
    {
        case 0: return BTN_CLICK;
        case 1: return BTN_PRESS;
        case 2: return BTN_RELEASE;
        default: return NO_BTN;
    }
}

static short keypad_get_btn_num(KeyState key)
{
    if (key >= BTN_RELEASE_BEGIN + BTN_COUNT)
        return -1;

    return key % BTN_COUNT;
}

// Передаваемая структура должна быть enum : short.
// При ошибке получения кнопки результат будет -1, который будет конвертирован в переданный T-enum.
// По этой причине необходимо обработать enum == -1, например, { NO_BTN = -1, ... }, иначе будет ошибка.
template<typename T>
static std::enable_if_t<std::is_enum_v<T>, T> keypad_get_btn(KeyState key) {
    return static_cast<T>(keypad_get_btn_num(key));
}

#endif // !MATRIX_KEYPAD_ENUM_H