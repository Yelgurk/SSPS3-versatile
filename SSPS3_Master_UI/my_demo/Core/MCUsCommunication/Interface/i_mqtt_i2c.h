#pragma once
#ifndef I_MQTT_I2C_H
#define I_MQTT_I2C_H

#ifdef DEV_SSPS3_RUN_ON_PLC
    #include "./i_arduino_i2c.h"
    #include "./i_mqtt.h"
#else
    #include "./i_arduino_i2c.h"
    #include "./i_mqtt.h"
#endif

static const constexpr size_t   MAX_PAYLOAD_SIZE_I2C = 2;
#define MPSI2C                  MAX_PAYLOAD_SIZE_I2C

// MqttCommandI2C – список доступных команд при работе по I2C (для SSPS3[F1/G4])
// CMD_END - лимит для команд по I2C, т.к. "старший" бит является "системным",
// т.е. только 7 из 8 бит доступно для инициализации команд
enum MqttCommandI2C : uint8_t
{
    CMD_NAN,
    ACK,
    NACK,
    GET_D_IO,
    GET_A_IN,
    GET_KB,
    SET_A_OUT,
    SET_R_OUT,
    CMD_END = 0b01111111
};

// Структура MqttMessageI2C – формат передаваемого сообщения
#pragma pack(push, 1)
struct MqttMessageI2C
{
    uint8_t command         = MqttCommandI2C::CMD_NAN;  // Команда (с флагом наличия следующих сообщений в старшем бите)
    uint8_t addr            = 0x02;                     // Адрес slave-а
    uint8_t content[MPSI2C] = {0x00};                   // Небольшой буфер данных как полезная нагрузка сообщения
    uint8_t seq             = 0;                        // Последовательный номер сообщения

    MqttMessageI2C()
    {}

    MqttMessageI2C(uint8_t command, uint8_t addr)
    {
        this->command = command;
        this->addr = addr;
    }

    // Заполнение поля content данными
    void set_content(const void* value_in, size_t length)
    {
        memset(content, 0, sizeof(content));
        memcpy(content, value_in, length);
    }

    // Заполнение поля content данными, размер которых соответствует типу T
    template<typename T>
    void set_content(void* value_in) {
        this->set_content(value_in, sizeof(T));
    }

    // Извлечение данных из content в переменную типа T
    template<typename T>
    void get_content(T* value_out) {
        memcpy(reinterpret_cast<uint8_t*>(value_out), content, sizeof(T));
    }

    // Установка флага наличия следующих сообщений
    MqttMessageI2C* set_has_a_following_messages(bool valid)
    {
        if (valid)  command |= 0b10000000;
        else        command &= 0b01111111;
        return this;
    }

    // Получение флага наличия следующих сообщений
    bool get_has_a_following_messages() const {
        return (command & 0b10000000) != 0;
    }

    // Получение команды без флага
    MqttCommandI2C get_command() const {
        return static_cast<MqttCommandI2C>(command & 0b01111111);
    }

    // Установка адреса получателя
    MqttMessageI2C* set_addr(uint8_t addr)
    {
        this->addr = addr;
        return this;
    }

    // Возвращает указатель на dummy-сообщение с заданным адресом
    static MqttMessageI2C* get_dummy(uint8_t addr)
    {
        static MqttMessageI2C dummy;
        return dummy.set_addr(addr);
    }

    // Размер структуры сообщения
    static uint8_t get_size_of()
    {
        static uint8_t _size_of = sizeof(MqttMessageI2C);
        return _size_of;
    }
};
#pragma pack(pop)

// Тип для обработчиков команд: функция, принимающая полученное сообщение
using AfterReceiveHandler = std::function<void(MqttMessageI2C)>;

// Структура подписчика/slave (на стороне мастера)
struct MqttSlaveSubscriber
{
    uint8_t address     = 0x02; // Адрес слейва
    char interrupt_pin  = -1;   // Пин для сигнала (уведомления о сообщениях)
    char restart_pin    = -1;   // Пин для вызова рестарта I2C на стороне слейва

    // Вектор пар: (команда, обработчик)
    std::vector<std::pair<uint8_t, AfterReceiveHandler>> command_handlers;
    
    MqttSlaveSubscriber() {}
    
    // Конструктор с дополнительным параметром restart_pin
    MqttSlaveSubscriber(uint8_t address, uint8_t interrupt_pin, uint8_t restart_pin = -1)
        : address(address), interrupt_pin(interrupt_pin), restart_pin(restart_pin)
    {}
};

class IMqttI2C : public IMqtt
{
public:
    IMqttI2C* get_this_imqtt_i2c() {
        return this;
    }

    virtual I2C_CH* begin(
        uint8_t sda,
        uint8_t scl,
        unsigned int freq,
        bool is_master,
        uint8_t addr = 0x01,
        char interrupt_pin = -1,
        int i2c_restart_call_pin = -1
    ) = 0;

    virtual I2C_CH* begin(
        I2C_CH* i2c_inited,
        uint8_t sda,
        uint8_t scl,
        unsigned int freq,
        bool is_master,
        uint8_t addr = 0x01,
        char interrupt_pin = -1,
        int i2c_restart_call_pin = -1
    ) = 0;

    // Регистрация обработчика для указанного кода команды.
    virtual bool register_handler(
        uint8_t cmd,
        AfterReceiveHandler handler,
        uint8_t address = 0x01
    ) = 0;

    // Регистрация slave-а со стороны master-a для регистрации обработчиков и
    // выполнения прослушивания поступающих сообщений
    virtual void subscribe_slave(
        uint8_t address,
        uint8_t master_to_slave_signal_pin,
        uint8_t master_to_slave_restart_pin = -1
    ) = 0;
};



#endif
