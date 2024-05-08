#include "../include/main.hpp"

#ifndef SSPS3_APPLICATION_SOLUTION

uint8_t FRAM_address = FRAM_I2C_ADDR;
TwoWire * itcw;

void setup()
{
    Serial.begin(115200);

    itcw = new TwoWire(0);
    itcw->begin(SDA, SCL, 400000);
}

uint8_t multipl = 0;

void loop()
{
    ++multipl;

    delay(5000);

    if (true)
    {
        Serial.print("is init:  ");
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

        if (!mem_Timer1.isInitialized()) mem_Timer1.unset();
        if (!mem_Timer2.isInitialized()) mem_Timer2.unset();
        if (!mem_Timer3.isInitialized()) mem_Timer3.unset();
        if (!mem_Timer4.isInitialized()) mem_Timer4.unset();
        if (!mem_Timer5.isInitialized()) mem_Timer5.unset();
    }

    if (true)
    {
        Serial.print("read old: ");
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
    }

    if (true)
    {
        Serial.print("set:      ");
        mem_Timer1.set(290 * multipl);
        mem_Timer2.set(290 * multipl);
        mem_Timer3.set(290 * 10 * multipl);
        mem_Timer4.set(290 * 10 * 2 * multipl);
        mem_Timer5.set(290 * 10 * 4 * multipl);

        Serial.print((boolean)(290 * multipl));
        Serial.print(" | ");
        Serial.print((uint8_t)(290 * multipl));
        Serial.print(" | ");
        Serial.print((uint16_t)(290 * 10 * multipl));
        Serial.print(" | ");
        Serial.print((uint32_t)(290 * 10 * 2 * multipl));
        Serial.print(" | ");
        Serial.print((uint32_t)(290 * 10 * 4 * multipl));
        Serial.println();
    }

    if (true)
    {
        Serial.print("read new: ");
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
    }

    if (true)
    {
        Serial.print("addr:     ");
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
    }
    
    Serial.println();
}

#endif