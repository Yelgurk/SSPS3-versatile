#include "../include/main.hpp"

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
    INTERRUPT_MASTER(0);
    switch (command)
    {
    case I2C_COMM::GET_RELAY_VAL:   return digitalRead(Relay[PIN_EXISTS(pin, Relay, uint8_t)]);
    case I2C_COMM::GET_DAC_VAL:     return analogRead(Dac[PIN_EXISTS(pin, Dac, uint8_t)]);
    case I2C_COMM::GET_DGIN_VAL:    return digitalRead(OptIn[PIN_EXISTS(pin, OptIn, uint8_t)]);
    case I2C_COMM::GET_ANIN_VAL:    return analogRead(Adc[PIN_EXISTS(pin, Adc, uint8_t)]);
    case I2C_COMM::GET_KB_VAL:      return KeyPressed;
    default:
        return 0;
    }
}

void finally_event(I2C_COMM command)
{
    if (command == I2C_COMM::STATE_STARTUP)
        INTERRUPT_MASTER(0);
        
    if (master_startup_success)
        wd_last_call_ms = millis();

    if (command == I2C_COMM::STATE_STARTUP && !master_startup_success)
    {
        master_startup_success = true;
        wd_curr_time_ms = wd_last_call_ms = millis();
    }
}

void try_interrupt_master()
{
    if (IS_MASTER_INTERRUPTED)
    {
        INTERRUPT_MASTER(0);
        delayMicroseconds(80);
    }

    INTERRUPT_MASTER(1);
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
    I2C = new I2C_Service(itcw, STM_I2C_ADR, set_event, get_event, finally_event);
}

void loop()
{
    if (master_startup_success && (wd_curr_time_ms = millis()) - wd_last_call_ms >= WATCHDOG_MS)
        NVIC_SystemReset();

    for (uint8_t pin = 0, state; pin < ARR_SIZE(OptIn, uint8_t); pin++)
    {
        state = digitalRead(OptIn[pin]);

        if (state != OptIn_state[pin])
            try_interrupt_master();

        OptIn_state[pin] = state;
    }

    if (kpd.getKeys())
        for (int i = 0; i < LIST_MAX; i++)
            if (kpd.key[i].stateChanged)
                switch (kpd.key[i].kstate)
                {
                    case PRESSED: {
                        KeyPressed = getposition(keysInline, KB_Col * KB_Row, kpd.key[i].kchar);
                        KeyHoldNext = millis() + (KeyHoldDelay = HOLD_begin_ms);
                        try_interrupt_master();
                    }; break;

                    case RELEASED: {
                        KeyPressed = KeyPressed == getposition(keysInline, KB_Col * KB_Row, kpd.key[i].kchar) ? KB_Await : KeyPressed; 
                        try_interrupt_master();
                    }; break;
                }

    if ((KeyPressed < KB_Col * KB_Row) && millis() >= KeyHoldNext)
    {
        KeyHoldDelay /= HOLD_x;
        KeyHoldDelay = KeyHoldDelay < HOLD_min_ms ? HOLD_min_ms : KeyHoldDelay;
        
        KeyHoldNext = millis() + KeyHoldDelay;
        try_interrupt_master();
    }
}