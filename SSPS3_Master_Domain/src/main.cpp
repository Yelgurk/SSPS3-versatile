#include "../include/main.hpp"

#ifdef UNLOCK_DOMAIN_MAIN_CPP

#define DEMO_DT         0
#define DEMO_FRAM_MEM   1

/*
1)
for any DEMO rename library.json to _library._json
for excluding compiler error

2) for compiling SSPS3_Master_Application
return name to library.json
*/

uint8_t FRAM_address = FRAM_I2C_ADDR;
TwoWire * itcw;
S_DateTime * dateTime;

uint16_t multipl = 0;
uint64_t ms_last_1 = 0,
         ms_last_2 = 0;

void setup()
{
    //Serial.begin(115200);
    Serial.begin(921600);

    itcw = new TwoWire(0);
    itcw->begin(SDA, SCL, 400000);

    Serial.println(("sizeof(S_Time) = " + to_string(sizeof(S_Time))).c_str());
    Serial.println(("sizeof(S_Date) = " + to_string(sizeof(S_Date))).c_str());
    Serial.println(("sizeof(S_DateTime) = " + to_string(sizeof(S_DateTime))).c_str());

    Serial.println(mem_DT.get().to_string().c_str());
    mem_DT.set(defaultDT);
    Serial.println(mem_DT.get().to_string().c_str());
    mem_DT.set((mem_DT.get() += S_Date(1, 0, 0, true)) -= S_Time(0, 0, 1));
    Serial.println(mem_DT.get().to_string().c_str());

    delay(5000);

    Serial.println(millis());
    uint32_t temp_buff = mem_Timer4.get();
    uint32_t test_ms = millis();
    uint32_t i;
    for (i = 0; millis() - test_ms <= 1000; i++)
    {
        mem_Timer4.get();
        mem_Timer4.set(i);
        mem_Timer4.get();
    }
    mem_Timer4.set(temp_buff);
    
    Serial.println(millis());
    Serial.println((std::string("done: ") + to_string(i) + " times").c_str());

    delay(5000);
    
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
    if (millis() - ms_last_1 >= 50)
    {
        if (true)
        {
            Serial.print("is init:  ");
            Serial.print(mem_Timer1.crc_state());
            Serial.print(" | ");
            Serial.print(mem_Timer2.crc_state());
            Serial.print(" | ");
            Serial.print(mem_Timer3.crc_state());
            Serial.print(" | ");
            Serial.print(mem_Timer4.crc_state());
            Serial.print(" | ");
            Serial.print(mem_Timer5.crc_state());
            Serial.print(" | ");
            Serial.print(mem_String.crc_state());
            Serial.print(" | ");
            Serial.print(mem_TDS_1.crc_state());
            Serial.print(" | ");
            Serial.print(mem_DT.crc_state());
            Serial.println();
            if (!mem_Timer1.crc_state()) mem_Timer1.unset();
            if (!mem_Timer2.crc_state()) mem_Timer2.unset();
            if (!mem_Timer3.crc_state()) mem_Timer3.unset();
            if (!mem_Timer4.crc_state()) mem_Timer4.unset();
            if (!mem_Timer5.crc_state()) mem_Timer5.unset();
            if (!mem_String.crc_state()) mem_String.unset();
            if (!mem_TDS_1.crc_state()) mem_TDS_1.unset();
            if (!mem_TDS_2.crc_state()) mem_TDS_2.unset();
            if (!mem_TDS_4.crc_state()) mem_TDS_4.unset();
            if (!mem_DT.crc_state()) mem_DT.unset();
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
            Serial.print(" | ");
            Serial.print(mem_String);
            Serial.print(" | ");
            Serial.print(mem_TDS_1.get().to_string().c_str());
            Serial.print(" | ");
            Serial.print(mem_DT.get().to_string().c_str());
            Serial.println();
        }

        if (true)
        {
            Serial.print("set:      ");
            mem_Timer1.set(!mem_Timer1.get());
            mem_Timer2.set(290 * multipl);
            mem_Timer3.set(290 * 10 * multipl);
            mem_Timer4.set(290 * 10 * 2 * multipl);
            mem_Timer5.set(290 * 10 * 4 * multipl);
            mem_String.set("inc = " + to_string(multipl));
            
            TaskDataStruct tds_edit = mem_TDS_1.get();
            tds_edit.done = !tds_edit.done;
            tds_edit.lap += 2;
            tds_edit.tempC += 5;
            tds_edit.ms_total += 10;
            tds_edit.ms_left += 20;

            Serial.print(mem_Timer1.get());
            Serial.print(" | ");
            Serial.print((uint8_t)(290 * multipl));
            Serial.print(" | ");
            Serial.print((uint16_t)(290 * 10 * multipl));
            Serial.print(" | ");
            Serial.print((uint32_t)(290 * 10 * 2 * multipl));
            Serial.print(" | ");
            Serial.print((uint32_t)(290 * 10 * 4 * multipl));
            Serial.print(" | ");
            Serial.print(("inc = " + to_string(multipl)).c_str());
            Serial.print(" | ");
            Serial.print(tds_edit.to_string().c_str());
            Serial.print(" | ");
            Serial.print((mem_DT.get() += S_Time(0, 0, 1)).to_string().c_str());
            Serial.println();

            mem_TDS_1.set(tds_edit);
            mem_DT.set(mem_DT.get() += S_Time(0, 0, 1));
        }

        if (true)
        {
            Serial.print("read new: ");
            Serial.print(mem_Timer1.get());
            Serial.print(" | ");
            Serial.print(mem_Timer2.get());
            Serial.print(" | ");
            Serial.print(mem_Timer3.get());
            Serial.print(" | ");
            Serial.print(mem_Timer4.get());
            Serial.print(" | ");
            Serial.print(mem_Timer5.get());
            Serial.print(" | ");
            Serial.print(mem_String.get().c_str());
            Serial.print(" | ");
            Serial.print(mem_TDS_1.get().to_string().c_str());
            Serial.print(" | ");
            Serial.print(mem_DT.get().to_string().c_str());

            Serial.println();
        }

        if (true)
        {
            /*
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
            Serial.print(" | ");
            Serial.print(mem_String.getAddress());
            Serial.print(" | ");
            Serial.print(mem_TDS_1.getAddress());
            Serial.println();
            */
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