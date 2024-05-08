#include "../include/main.hpp"

#define FRAM_DEMO_WRITE   1

uint8_t FRAM_address = FRAM_I2C_ADDR;
TwoWire * itcw;
STM32_slave * STM32;

volatile bool interrupted_by_slave = false;

void print(bool only_digital = false);

void setup()
{
    Serial.begin(115200);

    pinMode(INT, INPUT);
    attachInterrupt(INT, [](){ interrupted_by_slave = true; }, RISING);

    itcw = new TwoWire(0);
    STM32 = new STM32_slave(itcw, STM_I2C_ADDR, SDA, SCL);

    delay(5000);

    if (true)
    {
        Serial.print(mem_Timer1.isInitialized());
        Serial.print(" | ");
        Serial.print(mem_Timer2.isInitialized());
        Serial.print(" | ");
        Serial.print(mem_Timer3.isInitialized());
        Serial.print(" | ");
        Serial.print(mem_Timer4.isInitialized());
        Serial.print(" | ");
        Serial.print(mem_Timer5.isInitialized());
        Serial.println();
        Serial.println();

        if (!mem_Timer1.isInitialized()) mem_Timer1.unset();
        if (!mem_Timer2.isInitialized()) mem_Timer2.unset();
        if (!mem_Timer3.isInitialized()) mem_Timer3.unset();
        if (!mem_Timer4.isInitialized()) mem_Timer4.unset();
        if (!mem_Timer5.isInitialized()) mem_Timer5.unset();
    }

    if (true)
    {
        Serial.print(mem_Timer1);
        Serial.print(" | ");
        Serial.print(mem_Timer2);
        Serial.print(" | ");
        Serial.print(mem_Timer3);
        Serial.print(" | ");
        Serial.print(mem_Timer4);
        Serial.print(" | ");
        Serial.print(mem_Timer5);
        Serial.println();
        Serial.println();
    }

    if (true)
    {
        mem_Timer1.set(290);
        mem_Timer2.set(290);
        mem_Timer3.set(290 * 100);
        mem_Timer4.set(290 * 100 * 2);
        mem_Timer5.set(290 * 100 * 4);

        Serial.print((boolean)290);
        Serial.print(" | ");
        Serial.print((uint8_t)290);
        Serial.print(" | ");
        Serial.print((uint16_t)(290 * 100));
        Serial.print(" | ");
        Serial.print((uint32_t)(290 * 100 * 2));
        Serial.print(" | ");
        Serial.print((uint32_t)(290 * 100 * 4));
        Serial.println();
        Serial.println();
    }

    if (true)
    {
        Serial.print(mem_Timer1);
        Serial.print(" | ");
        Serial.print(mem_Timer2);
        Serial.print(" | ");
        Serial.print(mem_Timer3);
        Serial.print(" | ");
        Serial.print(mem_Timer4);
        Serial.print(" | ");
        Serial.print(mem_Timer5);
        Serial.println();
        Serial.println();
    }

    if (true)
    {
        Serial.print(mem_Timer1.getAddress());
        Serial.print(" | ");
        Serial.print(mem_Timer2.getAddress());
        Serial.print(" | ");
        Serial.print(mem_Timer3.getAddress());
        Serial.print(" | ");
        Serial.print(mem_Timer4.getAddress());
        Serial.print(" | ");
        Serial.print(mem_Timer5.getAddress());
        Serial.println();
        Serial.println();
    }
}

int pin = 0;
uint32_t ms_old = 0,
         ms_curr = 0,
         ms_old_2 = 0,
         ms_curr_2 = 0;

void loop()
{
    if (interrupted_by_slave)
    {
        interrupted_by_slave = false;

        print(true);

        for (uint8_t i = 0; i < 8; i++)
            STM32->set(COMM_SET::RELAY, i, OptIn_state[i]);
    }


    if ((ms_curr_2 = millis()) - ms_old_2 > 1000)
    {
        ms_old_2 = ms_curr_2;
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