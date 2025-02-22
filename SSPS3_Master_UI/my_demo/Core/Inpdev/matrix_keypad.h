#pragma once
#ifndef MATRIX_KEYPAD_H
#define MATRIX_KEYPAD_H

#ifdef DEV_SSPS3_RUN_ON_PLC
    #include <Arduino.h>
    #include <functional>
    #include "./matrix_keypad_enum.h"
#else
    #include <Arduino.h>
    #include <functional>
    #include "matrix_keypad_enum.h"
#endif

class MatrixKeypad
{
public:
    

private:
    MatrixKeypad() {}

    uint8_t rowPins[4];
    uint8_t colPins[4];
    bool send_press_once;
    bool in_press_state;
    bool notifyPressAsClick;
    uint16_t debounceDelay;
    uint16_t initialPressDelay;
    float pressAccelerationFactor;
    uint16_t minPressDelay;
    unsigned long pollInterval;

    int currentKeyIndex;         
    unsigned long pressStartTime;  
    unsigned long lastRepeatTime;  
    unsigned long lastScanTime;   

    volatile unsigned long lastEventTime;   
    volatile bool keyEventFlag;

    std::function<void(KeyState)> handler;

    // Вычисление задержки повторного события при удержании.
    uint16_t currentPressDelay()
    {
        unsigned long elapsed = millis() - pressStartTime;
        uint16_t delayVal = initialPressDelay;
        uint16_t repeats = elapsed / initialPressDelay;

        for (uint16_t i = 0; i < repeats; i++)
        {
            delayVal = (uint16_t)(delayVal * pressAccelerationFactor);

            if (delayVal < minPressDelay)
            {
                delayVal = minPressDelay;
                break;
            }
        }

        return delayVal;
    }

    int scanKey()
    {
        for (uint8_t row = 0; row < 4; row++)
        {
            digitalWrite(rowPins[row], HIGH);
            delayMicroseconds(5);

            for (uint8_t col = 0; col < 4; col++)
            {
                if (digitalRead(colPins[col]) == HIGH)
                {
                    digitalWrite(rowPins[row], LOW);
                    return row * 4 + col;
                }
            }

            digitalWrite(rowPins[row], LOW);
        }
        return -1;
    }

public:
    static MatrixKeypad* instance() {
        static MatrixKeypad inst;
        return &inst;
    }

    void begin(
        uint8_t r0, uint8_t r1, uint8_t r2, uint8_t r3,
        uint8_t c0, uint8_t c1, uint8_t c2, uint8_t c3,
        bool send_press_once = false)
    {
        rowPins[0] = r0; rowPins[1] = r1; rowPins[2] = r2; rowPins[3] = r3;
        colPins[0] = c0; colPins[1] = c1; colPins[2] = c2; colPins[3] = c3;
        this->send_press_once = send_press_once;
        this->in_press_state = in_press_state;

        for (uint8_t i = 0; i < 4; i++)
        {
            pinMode(rowPins[i], OUTPUT);
            digitalWrite(rowPins[i], LOW);
        }

        for (uint8_t i = 0; i < 4; i++)
        {
            pinMode(colPins[i], INPUT_PULLDOWN);
        }

        notifyPressAsClick      = false;
        debounceDelay           = 50;
        initialPressDelay       = 500;
        pressAccelerationFactor = 0.9f;
        minPressDelay           = 50;
        pollInterval            = 10;

        currentKeyIndex   = -1;
        pressStartTime    = 0;
        lastRepeatTime    = 0;
        lastScanTime      = 0;
        lastEventTime     = 0;

        keyEventFlag    = false;
    }

    KeyState update()
    {
        unsigned long now = millis();

        if (now - lastScanTime < pollInterval)
            return BTN_DELAY;

        lastScanTime = now;

        int detectedKey = scanKey();
        if (detectedKey >= 0)
        {
            if (now - lastEventTime < debounceDelay)
                return BTN_DELAY;

            lastEventTime = now;

            if (currentKeyIndex != -1 && detectedKey != currentKeyIndex)
            {
                KeyState retRelease = static_cast<KeyState>(BTN_RELEASE_0 + currentKeyIndex);
                
                if (handler)
                    handler(retRelease);

                currentKeyIndex = detectedKey;
                pressStartTime = now;
                lastRepeatTime = now;

                KeyState retClick = static_cast<KeyState>(BTN_CLICK_0 + currentKeyIndex);

                if (handler)
                    handler(retClick);

                in_press_state = false; // сбрасываем состояние удержания
                return retClick;
            }

            if (currentKeyIndex == -1)  // Обнаружение CLICK-а
            {
                currentKeyIndex = detectedKey;
                pressStartTime = now;
                lastRepeatTime = now;
                KeyState ret = static_cast<KeyState>(BTN_CLICK_0 + currentKeyIndex);

                if (handler)
                    handler(ret);
                return ret;
            }
            else // Обнаружение PRESSED
            {
                if (detectedKey != currentKeyIndex)
                    return BTN_DELAY;

                if (now - lastRepeatTime >= currentPressDelay())
                {
                    lastRepeatTime = now;
                    KeyState ret = notifyPressAsClick ?
                        static_cast<KeyState>(BTN_CLICK_0 + currentKeyIndex) :
                        static_cast<KeyState>(BTN_PRESS_0 + currentKeyIndex);

                    if (handler)
                        if (!send_press_once || !in_press_state)
                            handler(ret);

                    in_press_state = true;
                    return ret;
                }
                else
                {
                    return BTN_DELAY;
                }
            }
        }
        else // Обнаружение RELEASED или NO_BTN
        {
            if (currentKeyIndex != -1)
            {
                KeyState ret = static_cast<KeyState>(BTN_RELEASE_0 + currentKeyIndex);
                currentKeyIndex = -1;

                in_press_state = false;

                if (handler)
                    handler(ret);
                return ret;
            }
            return NO_BTN;
        }
    }

    unsigned long delay_debounce_calibration()
    {
        Serial.println("Calibration: Waiting for key press...");

        int key = -1;
        while ((key = scanKey()) < 0)
            delay(1);

        Serial.print("Key pressed: ");
        Serial.println(key);
        unsigned long startTime = millis();
        unsigned long continuousTime = 0;
        unsigned long lastTime = millis();

        while (continuousTime < 500)
        {
            int curKey = scanKey();
            unsigned long now = millis();

            if (curKey == key && curKey >= 0)
                continuousTime += (now - lastTime);
            else
                continuousTime = 0;

            lastTime = now;
            delay(5);
        }

        unsigned long totalTime = lastTime - startTime;
        unsigned long measuredDebounce = totalTime - 500;

        Serial.print("Calibration debounce time: ");
        Serial.print(measuredDebounce);
        Serial.println(" ms");
        Serial.print("Start: ");
        Serial.println(startTime);
        Serial.print("Finish: ");
        Serial.println(lastTime);
        

        return measuredDebounce;
    }

    void setPressRepeatAcceleration(uint16_t initialDelay, float accelerationFactor)
    {
        initialPressDelay = initialDelay;
        pressAccelerationFactor = accelerationFactor;
    }

    void setNotifyPressAsClick(bool enable) {
        notifyPressAsClick = enable;
    }
    
    void setMinimumPressDelay(uint16_t ms) {
        minPressDelay = ms;
    }

    void setDebounceDelay(uint16_t ms) {
        debounceDelay = ms;
    }

    void setHandler(std::function<void(KeyState)> func) {
        handler = func;
    }
};

#endif
