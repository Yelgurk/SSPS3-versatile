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
#ifdef ALLOW_UART_DEBUG
    Serial.setRx(S_UART_RX);
    Serial.setTx(S_UART_TX);
    Serial.begin(115200);
    
    Serial.println("KMA_05.02.2025_02:11:00");
#endif

    analogWriteResolution(12);
    analogReadResolution(12);

    pinMode(INT, OUTPUT);
    pinMode(INT_KB, OUTPUT);

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

    if (is_key_first_call_done && KeyPressed < KB_Size && KeyPressed == KeyNew && millis() >= KeyHoldNext)
    {
        KeyHoldDelay /= HOLD_x;
        KeyHoldDelay = KeyHoldDelay < HOLD_min_ms ? HOLD_min_ms : KeyHoldDelay;
        
        KeyHoldNext = millis() + KeyHoldDelay;
        is_key_press = true;
        CHANGE_INT_SIGNAL();
    }
#elif KeyPadVersion == 2
    char keypad_awaiter = myKeypad.getKeyWithDebounce();
    
    if (keypad_awaiter != Keypad4495::NO_PIN)
        run_kb_dispatcher(keypad_awaiter);
#endif
}

void _call_master_for_kb_read()
{
#ifdef SSPS3F1_BLACKOUT_EDITION
    CHANGE_KB_SIGNAL();
#else
    CHANGE_INT_SIGNAL();
#endif
}

void run_kb_dispatcher(char key)
{
    static bool _notify_slave_kb_await = false;

    // Преобразуем входное значение:
    // Если key равен 0 (NO_KEY), то считаем, что клавиша отпущена (KB_Await).
    // Иначе, поскольку библиотека возвращает значения 1..16, вычтем 1, чтобы диапазон стал 0..15.
    uint8_t newKey = (key == 0 ? KB_Await : key - 1);

    if (_notify_slave_kb_await)
    {
        if (KeyPressed < KB_Size)
        {
            KeyPressed += KB_Size;
            _call_master_for_kb_read();
            
            return;
        }
        else
        {
            _notify_slave_kb_await = false;
        
            KeyPressed = KB_Await;
            _call_master_for_kb_read();
            
            return;
        }
    }

    // Если пришло состояние отсутствия нажатия (то есть отпускание):
    if (newKey == KB_Await)
    {
        // Если ранее была зафиксирована нажатая клавиша (т.е. значение в диапазоне 0..15),
        // то сформируем событие отпускания: это будет значение "нажатая клавиша + 16".
        if (KeyPressed < KB_Size)
        {
            _notify_slave_kb_await = true;

            KeyPressed += KB_Size;   // Например, если KeyPressed было 7, станет 23.
            _call_master_for_kb_read();
            // Не сбрасываем KeyPressed сразу – сброс произойдёт при следующем вызове get_pressed_key()
            // согласно логике там (когда значение будет возвращено и затем сброшено в KB_Await).
        }

        return;
    }

    // Если получено валидное нажатие (newKey от 0 до 15):

    // Случай 1: Первое нажатие или состояние уже сброшено (или после release).
    if (KeyPressed == KB_Await || KeyPressed >= KB_Size)
    {
        KeyPressed = newKey;
        KeyHoldDelay = HOLD_begin_ms;
        KeyHoldNext = millis() + KeyHoldDelay;
        is_key_first_call_done = true;
        _call_master_for_kb_read();
    }
    // Случай 2: Если нажата другая клавиша, отличная от текущей.
    else if (KeyPressed < KB_Size && newKey != KeyPressed)
    {
        // Сначала посылаем событие отпускания предыдущей клавиши:
        KeyPressed += KB_Size; // теперь значение release (например, 7 -> 23)
        _call_master_for_kb_read();
        // Затем регистрируем новое нажатие:
        KeyPressed = newKey;
        KeyHoldDelay = HOLD_begin_ms;
        KeyHoldNext = millis() + KeyHoldDelay;
        is_key_first_call_done = true;
        _call_master_for_kb_read();
    }
    // Случай 3: Если удерживается та же клавиша.
    else if (KeyPressed < KB_Size && newKey == KeyPressed)
    {
        if (millis() >= KeyHoldNext)
        {
            KeyHoldDelay = KeyHoldDelay / HOLD_x;
            if (KeyHoldDelay < HOLD_min_ms)
                KeyHoldDelay = HOLD_min_ms;
            KeyHoldNext = millis() + KeyHoldDelay;
            _call_master_for_kb_read();
        }
    }
}