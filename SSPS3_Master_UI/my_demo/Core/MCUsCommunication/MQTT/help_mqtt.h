#pragma once
#ifndef HELP_MQTT_H
#define HELP_MQTT_H

// MASTER
/*
#include "../../SSPS3_Master_UI/my_demo/Core/MCUsCommunication/MQTT/mqtt_i2c.h"

void setup()
{
    static short _local_master_counter = 0;

    MqttI2C::instance()->begin(SDA, SCL, 400000, true);
    MqttI2C::instance()->subscribe_slave(STM_I2C_ADDR, INT, INT_KB);
    MqttI2C::instance()->set_use_ack_nack(true);
    MqttI2C::instance()->register_handler(
        MqttCommandI2C::GET_D_IO,
        [](MqttMessageI2C message)
        {
            short _received_slave_counter = 0;
            message.get_content<short>(&_received_slave_counter);

            Serial.println(_received_slave_counter);
        },
        STM_I2C_ADDR
    );

    while(1)
    {
        if(1)
        {
            _local_master_counter++;
            MqttI2C::instance()->push_message(GET_D_IO, &_local_master_counter, 2, STM_I2C_ADDR);
        }
        MqttI2C::instance()->update();
    }  
}
*/
// !MASTER

// SLAVE
/*
#include "../../SSPS3_Master_UI/my_demo/Core/MCUsCommunication/MQTT/mqtt_i2c.h"

void setup()
{
    static short _local_slave_counter = 0;

    MqttI2C::instance()->begin(SDA, SCL, 400000, false, STM_I2C_ADR, INT, INT_KB);
    MqttI2C::instance()->set_use_ack_nack(true);
    MqttI2C::instance()->register_handler(
        MqttCommandI2C::GET_D_IO,
        [](MqttMessageI2C message)
        {
            short _received_master_counter = 0;
            message.get_content<short>(&_received_master_counter);

            Serial.println(_received_master_counter);
        }
    );

    while(1)
    {
        if(1)
        {
            _local_slave_counter++;
            MqttI2C::instance()->push_message(GET_D_IO, &_local_slave_counter, 2);
        }
        MqttI2C::instance()->update();
    }
}
*/
// !SLAVE
#endif
