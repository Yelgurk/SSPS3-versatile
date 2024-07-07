#include "../include/main.hpp"

#ifdef UNLOCK_INFRASTRUCTURE_MAIN_CPP

#include "DS3231.h"

TwoWire * itcw;
STM32_slave * STM32;
DS3231 * rtc;
bool century = false;
bool h12Flag;
bool pmFlag;

void setup()
{
    Serial.begin(115200);

    pinMode(INT, INPUT);
    attachInterrupt(INT, [](){ interrupted_by_slave = true; }, CHANGE);

    itcw = new TwoWire(0);
    itcw->begin(SDA, SCL, 400000);

    STM32 = new STM32_slave(STM_I2C_ADDR);

    rtc = new DS3231(*itcw);
}

uint32_t ms_old = 0,
         ms_old_2 = 0,
         ms_curr = 0;

void loop()
{
    if (interrupted_by_slave)
    {
        interrupted_by_slave = false;

        print(true);

        for (uint8_t i = 0; i < 4; i++)
            STM32->set(COMM_SET::RELAY, i, OptIn_state[i]);
    }

    
    if (millis() - ms_old_2 > 10)
    {
        STM32->set(COMM_SET::DAC, 0, (millis() / 10) % 4096);
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
        Serial.print(STM32->get(COMM_GET::ANIN, 3));

        Serial.print(" | ");

        Serial.print(rtc->getYear(), DEC);
        Serial.print("-");
        Serial.print(rtc->getMonth(century), DEC);
        Serial.print("-");
        Serial.print(rtc->getDate(), DEC);
        Serial.print(" ");
        Serial.print(rtc->getHour(h12Flag, pmFlag), DEC); //24-hr
        Serial.print(":");
        Serial.print(rtc->getMinute(), DEC);
        Serial.print(":");
        Serial.println(rtc->getSecond(), DEC);
    }
    else
        Serial.println();
}

#endif