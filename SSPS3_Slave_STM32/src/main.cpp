#include "../include/main.hpp"

bool is_key_press = false;
uint8_t get_pressed_key()
{
    if (is_key_press)
    {
        is_key_press = false;
        return KeyPressed;
    }
    else
        return KB_Await;
}

void set_event(I2C_COMM command, uint8_t pin, uint16_t value)
{
    switch (command)
    {
    case I2C_COMM::SET_RELAY_VAL: digitalWrite(Relay[PIN_EXISTS(pin, Relay, uint8_t)], value); break;
    case I2C_COMM::SET_DAC_VAL: analogWrite(Dac[PIN_EXISTS(pin, Dac, uint8_t)], value > 4095 ? 4095 : value); break;
    default:
        break;
    }
}

uint16_t get_event(I2C_COMM command, uint8_t pin)
{
    switch (command)
    {
    case I2C_COMM::GET_RELAY_VAL:   return digitalRead(Relay[PIN_EXISTS(pin, Relay, uint8_t)]);
    case I2C_COMM::GET_DAC_VAL:     return analogRead(Dac[PIN_EXISTS(pin, Dac, uint8_t)]);
    case I2C_COMM::GET_DGIN_VAL:    return digitalRead(OptIn[PIN_EXISTS(pin, OptIn, uint8_t)]);
    case I2C_COMM::GET_ANIN_VAL:    return analogRead(Adc[PIN_EXISTS(pin, Adc, uint8_t)]);
    case I2C_COMM::GET_KB_VAL:      return get_pressed_key();
    default:
        return 0;
    }
}

void setup()
{
    Serial.begin(115200);
    analogWriteResolution(12);
    analogReadResolution(12);

    pinMode(INT, OUTPUT);

    for(uint8_t pin : Relay)
    {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, 0);
    }

    for(uint8_t pin : Dac)
    {
        pinMode(pin, OUTPUT);
        analogWrite(pin, 0);
    } 

    for(uint8_t pin : OptIn)
        pinMode(pin, INPUT_PULLDOWN);

    for(uint8_t pin : Adc)
        pinMode(pin, INPUT_PULLDOWN);

    itcw = new TwoWire(SDA, SCL);
    I2C = new I2C_Service(itcw, STM_I2C_ADR, set_event, get_event);
}

uint8_t changed;
void loop()
{
    if (I2C->i2c_master_runned)
        if (I2C->i2c_silent_cnt >= MASTER_AWAIT_LIMIT_MS / MASTER_INT_DEL)
            NVIC_SystemReset();

    if (millis() - I2C->i2c_last_call_ms >= MASTER_INT_DEL)
    {
        I2C->i2c_last_call_ms = millis();
        I2C->i2c_silent_cnt++;
        CHANGE_INT_SIGNAL();
    }

    changed = 0;
    for (uint8_t pin = 0, state; pin < ARR_SIZE(OptIn, uint8_t); pin++)
    {
        state = digitalRead(OptIn[pin]);

        if (state != OptIn_state[pin])
            changed++;

        OptIn_state[pin] = state;
    }

    if (changed > 0)
        CHANGE_INT_SIGNAL();

    if (kpd.getKeys())
        for (int i = 0; i < LIST_MAX; i++)
            if (kpd.key[i].stateChanged)
                switch (kpd.key[i].kstate)
                {
                    case PRESSED: {
                        KeyPressed = getposition(keysInline, KB_Col * KB_Row, kpd.key[i].kchar);
                        KeyHoldNext = millis() + (KeyHoldDelay = HOLD_begin_ms);
                        is_key_press = true;
                        CHANGE_INT_SIGNAL();
                    }; break;

                    case RELEASED: {
                        KeyPressed = KeyPressed == getposition(keysInline, KB_Col * KB_Row, kpd.key[i].kchar) ? KB_Await : KeyPressed; 
                        is_key_press = true;
                        CHANGE_INT_SIGNAL();
                    }; break;
                }

    if ((KeyPressed < KB_Col * KB_Row) && millis() >= KeyHoldNext)
    {
        KeyHoldDelay /= HOLD_x;
        KeyHoldDelay = KeyHoldDelay < HOLD_min_ms ? HOLD_min_ms : KeyHoldDelay;
        
        KeyHoldNext = millis() + KeyHoldDelay;
        is_key_press = true;
        CHANGE_INT_SIGNAL();
    }
}