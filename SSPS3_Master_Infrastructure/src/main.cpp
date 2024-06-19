#include "../include/main.hpp"

#ifndef SSPS3_APPLICATION_SOLUTION
#ifndef SSPS3_UI_DEV_SOLUTION

TwoWire * itcw;
STM32_slave * STM32;

void setup()
{
    Serial.begin(115200);

    pinMode(INT, INPUT);
    attachInterrupt(INT, [](){ interrupted_by_slave = true; }, RISING);

    itcw = new TwoWire(0);
    itcw->begin(SDA, SCL, 400000);

    STM32 = new STM32_slave(STM_I2C_ADDR);
}

uint32_t ms_old = 0,
         ms_curr = 0;

void loop()
{
    if (interrupted_by_slave)
    {
        interrupted_by_slave = false;

        print(true);

        for (uint8_t i = 0; i < 8; i++)
            STM32->set(COMM_SET::RELAY, i, OptIn_state[i]);
    }

    if ((ms_curr = millis()) - ms_old > 1000)
    {
        ms_old = ms_curr;
        print();
    }
}

void print(bool only_digital)
{
    Serial.print(STM32->get_kb());

    Serial.print(" | ");

    Serial.print(OptIn_state[0] = STM32->get(COMM_GET::DGIN, 0));
    Serial.print(" ");
    Serial.print(OptIn_state[1] = STM32->get(COMM_GET::DGIN, 1));
    Serial.print(" ");
    Serial.print(OptIn_state[2] = STM32->get(COMM_GET::DGIN, 2));
    Serial.print(" ");
    Serial.print(OptIn_state[3] = STM32->get(COMM_GET::DGIN, 3));
    Serial.print(" ");
    Serial.print(OptIn_state[4] = STM32->get(COMM_GET::DGIN, 4));
    Serial.print(" ");
    Serial.print(OptIn_state[5] = STM32->get(COMM_GET::DGIN, 5));
    Serial.print(" ");
    Serial.print(OptIn_state[6] = STM32->get(COMM_GET::DGIN, 6));
    Serial.print(" ");
    Serial.print(OptIn_state[7] = STM32->get(COMM_GET::DGIN, 7));
    
    if (!only_digital)
    {
        Serial.print(" | ");

        Serial.print(STM32->get(COMM_GET::ANIN, 0));
        Serial.print(" ");
        Serial.print(STM32->get(COMM_GET::ANIN, 1));
        Serial.print(" ");
        Serial.print(STM32->get(COMM_GET::ANIN, 2));
        Serial.print(" ");
        Serial.println(STM32->get(COMM_GET::ANIN, 3));
    }
    else
        Serial.println();
}

#endif
#endif