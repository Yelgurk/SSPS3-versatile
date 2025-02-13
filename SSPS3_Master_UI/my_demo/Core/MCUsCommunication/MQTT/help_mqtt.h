#pragma once
#ifndef HELP_MQTT_H
#define HELP_MQTT_H

// MASTER
/*

#include "MqttI2C.h"

void onSlaveResponse(const MqttMessage &msg)
{
    Serial.print("Master получил ответ от slave, cmd: 0x");
    Serial.println(msg.cmd, HEX);

    // Например, если полезная нагрузка – float:
    float receivedValue;
    memcpy(&receivedValue, msg.payload, sizeof(float));

    Serial.print("Значение: ");
    Serial.println(receivedValue);
}

void setup()
{
...

    Serial.begin(115200);

    // Инициализация I2C на ESP32 (пины заданы согласно схеме)
    // Допустим, пины SDA = 40, SCL = 39
    TwoWire * i2c = MqttI2C::getInstance().begin(true, 0x30); // 0x30 – адрес slave

    // Если есть пин уведомления от slave, его можно задать:
    MqttI2C::getInstance().setSlaveNotifyPin(38);

    // Задание адреса для master (например, 0x01)
    MqttI2C::getInstance().setAddress(0x01);

    // Регистрация обработчика для входящих сообщений (например, cmd = 0x10)
    MqttI2C::getInstance().registerHandler(0x10, onSlaveResponse);

    // Установка глобальных параметров (если требуется)
    MqttI2C::getInstance().setGlobalAckTimeout(150);
    MqttI2C::getInstance().setGlobalMaxRetries(3);

    // Пример: отправляем значение float 3.14 на slave с адресом 0x30 и командой 0x01
    MqttI2C::getInstance().queueMessage(0x30, 0x01, 3.14f);

...
}

void loop()
{
...

    MqttI2C::getInstance().update();
    delay(10);

...
}

*/
// !MASTER

//-----------------------------------------------------------------

// SLAVE
/*

#include "MqttI2C.h"

void onMasterCommand(const MqttMessage &msg)
{
    Serial.print("Slave получил команду 0x");
    Serial.println(msg.cmd, HEX);

    // Допустим, master передал значение float – выводим его:
    float val;
    memcpy(&val, msg.payload, sizeof(float));

    Serial.print("Полученное значение: ");
    Serial.println(val);

    // Можно подготовить ответ: отправляем обратно значение (например, val*2)
    float responseValue = val * 2;
    MqttI2C::getInstance().queueMessage(0x01, 0x10, responseValue); // 0x01 – адрес master, 0x10 – код ответа
}

void setup()
{
    ...

    Serial.begin(115200);

    // Инициализация I2C на ESP32 (пины заданы согласно схеме)
    // Допустим, пины SDA = 40, SCL = 39
    TwoWire* i2c = MqttI2C::getInstance().begin(false, 0x30, 6);
    
    // Задание собственного адреса
    MqttI2C::getInstance().setAddress(0x30);
    
    // Регистрация обработчика для входящих команд (например, cmd = 0x01)
    MqttI2C::getInstance().registerHandler(0x01, onMasterCommand);
    
    // Можно установить глобальные параметры
    MqttI2C::getInstance().setGlobalAckTimeout(150);
    MqttI2C::getInstance().setGlobalMaxRetries(3);

    ...
}

void loop()
{
    ...

    // Здесь в любой момент (например, по событию или условию) можно подготовить сообщение для мастера.
    // Например, если произошёл какой-то внутренний событие, и slave должен отправить данные мастеру:
    static bool messageSent = false;
    if (!messageSent &&  какое-то условие, например, событие произошло == true)
    {
    // Подготавливаем отправку сообщения: например, отправляем float значение 2.71
    // dst = 0x01 (адрес мастера), cmd = 0x10 (код сообщения, выбран произвольно)
    MqttI2C::getInstance().queueMessage(0x01, 0x10, 2.71f);
    // После вызова queueMessage() сообщение попадает в очередь slave
    // и сохраняется во внутреннем буфере (_pending_message).
    // Если notify-пин настроен, то slave выставит сигнал, чтобы мастер узнал, что данные готовы.
    messageSent = true;
    }

    // Вызов update() асинхронно обрабатывает отправку (updateSend) и приём (updateReceive) сообщений
    MqttI2C::getInstance().update();

    // Остальной код slave выполняется здесь...
    delay(10);

    ...
}

Объяснение кода:

1)
Подготовка отправки:
В loop() мы проверяем условие (в примере оно просто true для демонстрации). Когда условие выполнено, вызывается метод

> MqttI2C::getInstance().queueMessage(0x01, 0x10, 2.71f);

Это приводит к тому, что на стороне slave формируется сообщение (структура MqttMessage) с адресом отправителя (slave),
адресом получателя (мастера, 0x01), командой (0x10) и полезной нагрузкой (float 2.71).
Сообщение сохраняется в очереди _send_queue, а метод updateSend() затем обработает его.
В режиме slave sendMessageInternal() просто сохраняет сообщение в _pending_message.

2)
Уведомление мастера:
Если для slave настроен notify-пин (в данном случае пин 6), то после подготовки сообщения
метод updateSend() выставит этот пин (digitalWrite), сигнализируя мастеру, что данные готовы к считыванию.
Мастер должен, в свою очередь, опросить slave (например, через Wire.requestFrom или через обработчик прерывания, если настроен notify-пин).

3)
"Асинхронная" обработка:
Метод update() вызывается в loop() и быстро проверяет очередь сообщений для отправки и наличие полученных пакетов.
При этом основной поток "не блокируется".

*/
// !SLAVE
#endif
