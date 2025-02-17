#pragma once
#ifndef HELP_X_VAR_H
#define HELP_X_VAR_H

/*
// забавно. в какой-то момент устал писать комменты на английском. вот-так рекс я

#include "../my_demo/Core/Memory/x_var_data_center.h"
#include "../my_demo/Core/Memory/Device/mem_i2c_fm24gl64.h"

void _display_10_x_10_eeprom()
{
    for (unsigned char col = 0; col < 10; col++)
    {
        for (unsigned char row = 0; row < 10; row++)
        {
            unsigned char _address = col * 10 + row;
            char x = XStorageDispatcher->get_i_read_write()->byte_read(_address);
            Serial.printf("%03d ", x);
        }
        Serial.println();
    }
    Serial.println();
}

setup()
{
    itcw    = new TwoWire(0);
    itcw    ->begin(SDA, SCL, 400000);

    FM24GL64::set_i2c_ch(itcw);
    FM24GL64 _ee_bank_1(0x50);
    FM24GL64 _ee_bank_2(0x57);

    delay(15000);

    XStorageDispatcher->add_device(_ee_bank_1.get_i_read_write());
    XStorageDispatcher->add_device(_ee_bank_2.get_i_read_write());
    
    XStorageDispatcher->get_i_read_write()->fill(0, 0, 2000);
    _display_10_x_10_eeprom();

    Serial.println();
    Serial.print("MCS: ");
    Serial.println(XStorage->Startup_checksum.get().c_str());

    Serial.print("DCS: ");
    Serial.println(XStorage->Startup_checksum.get_default().c_str());
    Serial.println();

    if (!XStorage->Startup_checksum.is_equal_to_default())
        XAllocator->vars_full_hard_reset();
    _display_10_x_10_eeprom();

    unsigned char lol = 123;

    // set 15 into var (both eeprom will save (uint16_t)15 + (uint8_t)crc)
    XStorage->TV_short_arr[1] = 93;
    //XStorage->TestVar1 = 15;
    _display_10_x_10_eeprom();


    // broke crc for "TestVar1" on one of both equal records on both banks
    _ee_bank_1.write(26, &lol, sizeof(lol)); 
    _display_10_x_10_eeprom();


    // call "load from external mem" for reading + comparing records from both banks and pulling valid one into var (+ rewrite "error banks" with valid data)
    XStorage->TV_short_arr[1].load_from_ext_mem();
    //XStorage->TestVar1.load_from_ext_mem();
    _display_10_x_10_eeprom();


    _ee_bank_1.write(26, &lol, sizeof(lol)); // broke crc
    _ee_bank_2.write(26, &lol, sizeof(lol)); // for both banks (p.s. for checking "set default value")
    _display_10_x_10_eeprom();


    // call "load from extrenal mem" and "set default value" will be called (p.s. default == value on init)
    XStorage->TV_short_arr[1].load_from_ext_mem();
    //XStorage->TestVar1.load_from_ext_mem(); 
    _display_10_x_10_eeprom();

    while(1)
    {}  
}

*/

#endif // !HELP_X_VAR_H