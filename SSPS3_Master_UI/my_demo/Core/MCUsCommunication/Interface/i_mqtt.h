#pragma once
#ifndef I_MQTT_H
#define I_MQTT_H

#ifdef DEV_SSPS3_RUN_ON_PLC
    #include "../Array/array_ring_buffer.h"
    #include <functional>
    #include <vector>
#else
    #include "./array_ring_buffer.h"
    #include <functional>
    #include <vector>
#endif

class IMqtt
{
protected:
    unsigned short _max_messages_per_subscription = 64;
    bool _use_ack_nack = false;

public:
    virtual ~IMqtt() {}

    void set_max_messages_per_subscription(unsigned short max_messages_per_subscription) {
        this->_max_messages_per_subscription = max_messages_per_subscription;
    }

    void set_use_ack_nack(bool flag) {
        _use_ack_nack = flag;
    }

    IMqtt* get_this_imqtt() {
        return this;
    }

    virtual bool push_message(
        uint8_t cmd,
        const void* data,
        uint8_t length,
        uint8_t dst,
        uint8_t max_retries_spi = 0,
        unsigned long ack_timeout_spi = 0
    ) = 0;

    virtual void update() = 0;
};

#endif // I_MQTT_H
