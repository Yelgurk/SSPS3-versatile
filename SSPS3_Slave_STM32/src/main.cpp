#include "../include/main.hpp"

#if KeyPadVersion == 1
    bool is_key_press = false;
#endif

bool is_key_first_call_done = false;
bool is_key_press_rel_await = false;
uint8_t key_feedback = KB_Await;

uint8_t get_pressed_key()
{
#if KeyPadVersion == 1
    if (KeyPressed >= KB_Size && KeyPressed < KB_Await)
        is_key_press_rel_await = false;

    if (is_key_press)
    {
        is_key_press = false;
        return KeyPressed;
    }
    else
        return KB_Await;
#elif KeyPadVersion == 2
    key_feedback = KeyPressed;

    if (key_feedback < KB_Size)
        is_key_first_call_done = true;
    
    if (!is_key_first_call_done && key_feedback >= KB_Size && key_feedback < KB_Await)
        return (key_feedback = KeyPressed = KB_Await);

    if (is_key_press_rel_await || (key_feedback >= KB_Size && key_feedback < KB_Await))
    {
        is_key_press_rel_await = false;
        is_key_first_call_done = false;
        KeyPressed = KB_Await;
        return key_feedback;
    }
    
    return key_feedback;
#endif
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
        pinMode(pin, INPUT_ANALOG);

#if KeyPadVersion == 2
    for (uint8_t pin : KB_col_pin)
        pinMode(pin, OUTPUT);

    for (uint8_t pin : KB_row_pin)
        pinMode(pin, INPUT_PULLUP);

    myKeypad.setDebounce(150);
#endif

    itcw = new TwoWire(SDA, SCL);
    I2C = new I2C_Service(itcw, STM_I2C_ADR, set_event, get_event);
}

void run_kb_dispatcher(char key);

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

#if KeyPadVersion == 1
    if (!is_key_press_rel_await && kpd.getKeys())
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
                        KeyPressed = KeyPressed == getposition(keysInline, KB_Col * KB_Row, kpd.key[i].kchar) ? KeyPressed + KB_Size : KeyPressed; //KB_Await : KeyPressed; 
                        is_key_press = true;

                        if (KeyPressed >= KB_Size && KeyPressed < KB_Await)
                            is_key_press_rel_await = true;
                        CHANGE_INT_SIGNAL();
                    }; break;
                }
#elif KeyPadVersion == 2
    char keypad_awaiter = myKeypad.getKeyWithDebounce();
    
    if (keypad_awaiter >= 0)
        run_kb_dispatcher(keypad_awaiter);
#endif

    if (is_key_first_call_done && KeyPressed < KB_Size && KeyPressed == KeyNew && millis() >= KeyHoldNext)
    {
        KeyHoldDelay /= HOLD_x;
        KeyHoldDelay = KeyHoldDelay < HOLD_min_ms ? HOLD_min_ms : KeyHoldDelay;
        
        KeyHoldNext = millis() + KeyHoldDelay;
#if KeyPadVersion == 1
        is_key_press = true;
#endif
        CHANGE_INT_SIGNAL();
    }
}

void run_kb_dispatcher(char key)
{
    KeyNew = key;
    KeyNew = KeyNew == 0 ? KB_Await : KeyNew - 1;

    if (KeyNew != KB_Await)
    {
        if (KeyPressed == KB_Await)
        {
            KeyHoldNext = millis() + (KeyHoldDelay = HOLD_begin_ms);

            KeyPressed = KeyNew;
            CHANGE_INT_SIGNAL();
        }
        else if (KeyPressed < KB_Size && KeyPressed != KeyNew)
        {
            KeyHoldDelay = HOLD_begin_ms;

            KeyPressed = KeyPressed + KB_Size;
            is_key_press_rel_await = true;
            CHANGE_INT_SIGNAL();
        }
    }
    else if (KeyPressed < KB_Size)
    {
        KeyHoldDelay = HOLD_begin_ms;

        KeyPressed = KeyPressed + KB_Size;
        is_key_press_rel_await = true;
        CHANGE_INT_SIGNAL();
    }
}

/*
const char KEY_DEBOUNCE_AWAIT = -1;
unsigned long _lastDebounceTime = 0;
char _lastKey = Keypad4495::NO_KEY;
bool _debouncing = false;

char Keypad4495::getKeyWithDebounce() {
    char button = getKey();
    unsigned long currentMillis = millis();

    if (_debouncing)
    {
        if (currentMillis - _lastDebounceTime >= _debounce_ms)
        {
            // Если прошло достаточно времени, проверяем, совпадает ли кнопка
            _debouncing = false;
            
            if (button == _lastKey)
            {
                _lastKey = NO_KEY;
                return button; // Возвращаем кнопку, если она совпадает
            }
            else
            {
                return NO_KEY; // Игнорируем, если кнопка изменилась
            }
        }

        return KEY_DEBOUNCE_AWAIT; // Пока не истек debounce, возвращаем NO_KEY
    }

    if (button != NO_KEY && button != _lastKey) {
        // Начинаем debounce, если кнопка изменилась
        _debouncing = true;
        _lastDebounceTime = currentMillis;
        _lastKey = button;
        return KEY_DEBOUNCE_AWAIT; // Пока debounce активен, возвращаем NO_KEY
    }

    return NO_KEY; // Если кнопка не нажата или не изменилась
}
*/