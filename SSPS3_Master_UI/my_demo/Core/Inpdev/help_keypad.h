#pragma once 
#ifndef HELP_KEYPAD_H
#define HELP_KEYPAD_H

/*
#include "../../SSPS3_Master_UI/my_demo/Core/Inpdev/matrix_keypad.h"

void setup()
{
    MatrixKeypad* KP = MatrixKeypad::instance();

    KP->begin(
        KB_row_pin[0],
        KB_row_pin[1],
        KB_row_pin[2],
        KB_row_pin[3],
        KB_col_pin[0],
        KB_col_pin[1],
        KB_col_pin[2],
        KB_col_pin[3]
    );
    KP->setPressRepeatAcceleration(500, 0.85f);
    KP->setNotifyPressAsClick(false);
    KP->setMinimumPressDelay(50);
    KP->setDebounceDelay(20);
    KP->setHandler([](MatrixKeypad::KeyState key) {
        Serial.println(key);
    });

    //KP->delay_debounce_calibration();

    while(1)
    {
        KP->update();
    }
}
*/

#endif // !HELP_KEYPAD_H