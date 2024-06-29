#ifndef SSPS3_APPLICATION_SOLUTION

#include "../include/main.hpp"

#define DEMO_DT         0
#define DEMO_FRAM_MEM   1

/*
1)
for DEMO_DT rename library.json to _library._json
for excluding compiler error

2) for compiling SSPS3_Master_Application
return name to library.json
*/

uint8_t FRAM_address = FRAM_I2C_ADDR;
TwoWire * itcw;
S_DateTime * dateTime;

uint8_t multipl = 0;
uint64_t ms_last_1 = 0,
         ms_last_2 = 0;

void setup()
{
    Serial.begin(115200);

    itcw = new TwoWire(0);
    itcw->begin(SDA, SCL, 400000);

#if DEMO_DT == 1
    S_Date date(28, 2, 2024); // Valid leap year date
    S_Time time(14, 30, 0);   // 14:30:00
    dateTime = new S_DateTime(date, time);
    
    Serial.println(("DateTime: " + dateTime->to_string()).c_str());
    
    dateTime->set_date(S_Date(23, 6, 2024));
    dateTime->set_time(S_Time(
        23, 58, 10,
        [&]() { dateTime->get_date()->set_day(dateTime->get_date()->get_day() - 1); },
        [&]() { dateTime->get_date()->set_day(dateTime->get_date()->get_day() + 1); })
    );

    Serial.println(("Updated DateTime: " + dateTime->to_string()).c_str());
    
    S_DateTime addedDateTime = *dateTime + S_Time(2, 30, 0);
    
    Serial.println(("DateTime + 2:30:00: " + addedDateTime.to_string()).c_str());
#endif
}

void loop()
{
#if DEMO_FRAM_MEM == 1
    if (millis() - ms_last_1 >= 10000)
    {
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
        
        ms_last_1 = millis();
        ++multipl;

        Serial.println();
    }
#endif

#if DEMO_DT == 1
    if (millis() - ms_last_2 >= 50)
    {
        ms_last_2 = millis();
        *dateTime += S_Time(0, 0, 1);
        //*dateTime -= S_Date(1, 0, 0, true);

        Serial.println(("New DateTime: " + dateTime->to_string()).c_str());
    }
#endif
}

#endif