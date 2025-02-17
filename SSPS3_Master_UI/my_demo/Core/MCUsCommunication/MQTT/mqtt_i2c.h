#pragma once
#ifndef MQTT_I2C_H
#define MQTT_I2C_H

#ifdef DEV_SSPS3_RUN_ON_PLC
  #include "../Interface/i_mqtt_i2c.h"
#else
  #include "./i_mqtt_i2c.h"
#endif

#define INCOMING_BUFFER_SIZE 64
#define OUTGOING_BUFFER_SIZE 64

class MqttI2C : public IMqttI2C
{
private:
    // Приватный конструктор (синглтон)
    MqttI2C() {}

    // Подписки на стороне мастера: ключ – адрес слейва
    std::unordered_map<uint8_t, MqttSlaveSubscriber> _master_side_subscriptions;

    // Обработчики на стороне слейва
    std::vector<std::pair<uint8_t, AfterReceiveHandler>> _slave_side_command_handlers;
    
    // Кольцевые буферы для входящих и исходящих сообщений
    ArrayRingBuffer<MqttMessageI2C, INCOMING_BUFFER_SIZE> _mqtt_incoming_buffer;
    ArrayRingBuffer<MqttMessageI2C, OUTGOING_BUFFER_SIZE> _mqtt_outgoing_buffer;

    // Всё связанное с инициализацией и работой I2C
    I2C_CH* _i2c = nullptr;        // Интерфейс I2C
    uint8_t _sda = 0, _scl = 0;     // Пины I2C
    unsigned int _freq = 0;         // Частота работы I2C
    uint8_t _address = 0x01;        // Адрес устройства
    bool _is_master = false;        // Режим работы (мастер/слейв)
    short _interrupt_pin = -1;      // Пин для сигнала о наличии сообщений (используется на стороне слейва)
    short _i2cRestartCallPin = -1;  // Пин для сигнала о необходимости рестарта I2C (используется на стороне мастера)
    
    // ACK/NACK (сторона master-а): при отправке от master к slave сообщение остается в ожидании подтверждения
    // (при отключенном _use_ack_nack не используются)
    std::unordered_map<uint8_t, MqttMessageI2C> _master_pending_messages;   // Ключ – адрес slave
    std::unordered_map<uint8_t, unsigned long> _master_pending_timestamps;  // Время отправки для таймаута
    uint8_t _master_seq_counter = 0;                                        // Счётчик последовательных номеров для master

    // ACK/NACK (сторона slave-а): для сообщений, отправляемых от slave к master, необходимо сохранять pending-сообщение,
    // пока master не пришлёт ACK. Также pending-ответ slave для подтверждения получения от master.
    bool _slave_has_pending_outgoing = false;
    bool _slave_has_pending_response = false;
    MqttMessageI2C _slave_pending_outgoing;
    MqttMessageI2C _slave_pending_response;
    unsigned long _ackTimeout = 1000; // Таймаут ожидания ACK (для master->slave) в мс

    // Независимо от вызванного begin() сохраняем все данные об i2c шине
    // пригодится для случаев, когда необходимо будет выполнить рестарт i2c
    // с обеих сторон (master-a и slave-ов) при потере связи
    void _init_save_i2c_data(
        uint8_t sda,
        uint8_t scl,
        unsigned int freq,
        bool is_master,
        uint8_t addr,
        char interrupt_pin,
        int i2c_restart_call_pin
    )
    {
        this->_sda = sda;
        this->_scl = scl;
        this->_freq = freq;
        this->_is_master = is_master;
#ifdef DEV_SSPS3_IS_MASTER
        this->_address = addr;
#else
        this->_address = (addr <= 0x01) ? 0x02 : addr;
#endif
        this->_interrupt_pin = interrupt_pin;
        this->_i2cRestartCallPin = i2c_restart_call_pin;
    }

    // настройка логики "прерываний" для slave
    void _init_slave_i2c_interrupts()
    {
#ifdef DEV_SSPS3_IS_MASTER
#else
        if (_interrupt_pin != -1)
        {
            pinMode(_interrupt_pin, OUTPUT);
            digitalWrite(_interrupt_pin, LOW);
        }
        
        if (_i2cRestartCallPin != -1)
        {
            pinMode(_i2cRestartCallPin, INPUT_PULLUP);
            attachInterrupt(
                digitalPinToInterrupt(_i2cRestartCallPin),
                static_i2cRestartISR,
                FALLING
            );
        }

        this->_i2c->onReceive(static_on_receive);
        this->_i2c->onRequest(static_on_request);
#endif
    }

protected:
    // Callback обработки события onReceive для I2C (вызывается в прерывании)
    static void static_on_receive(int numBytes)
    {
        static MqttI2C* inst = MqttI2C::instance();
        static uint8_t msgSize = MqttMessageI2C::get_size_of();

        // Читаем все доступные байты
        while (inst->_i2c->available() >= msgSize)
        {
            MqttMessageI2C incoming_message;
            inst->_i2c->readBytes(reinterpret_cast<uint8_t*>(&incoming_message), msgSize);

            // Если режим включён, проверяем, является ли полученное сообщение ACK/NACK
            if (inst->_use_ack_nack)
            {
                // Если команда ACK или NACK, то это ответ от master на ранее отправленное сообщение slave
                if (incoming_message.get_command() == MqttCommandI2C::ACK || incoming_message.get_command() == MqttCommandI2C::NACK)
                {
                    // Сравниваем seq с pending-сообщением
                    if (inst->_slave_has_pending_outgoing && inst->_slave_pending_outgoing.seq == incoming_message.seq)
                    {
                        if (incoming_message.get_command() == MqttCommandI2C::ACK)
                        {
                            // ACK получен – удаляем pending-сообщение (будет замещено другим сообщением, далее, просто флаг)
                            inst->_slave_has_pending_outgoing = false;

                            // Не уверен, что этот блок проверки здесь нужен
                            // т.к. в случае пустого буфера сообщений у slave-а, самому
                            // master-у будет отправлен пакет-пустышка, который выйдет из блока с requestFrom у master-а
                            if (inst->_mqtt_outgoing_buffer.count() == 0)
                            {
                                inst->_set_interrupt_signal(false);
                            }
                        }
                        // Если получен NACK – оставляем pending, чтобы повторно отсылать
                    }
                }
                else
                {
                    // Если получено обычное сообщение от master, обрабатываем его
                    for (auto &handler_pair : inst->_slave_side_command_handlers)
                    {
                        if (handler_pair.first == incoming_message.get_command()) {
                            handler_pair.second(incoming_message);
                        }
                    }
                    
                    // Затем сразу формируем pending-ответ для master
                    MqttMessageI2C response;
                    response.command = MqttCommandI2C::ACK; // Если требуется можно добавить логику NACK при ошибке
                    response.addr = 0x01;   // Предполагаемый адрес мастера
                    response.seq = incoming_message.seq;
                    
                    noInterrupts();
                    inst->_slave_pending_response = response;
                    inst->_slave_has_pending_response = true;
                    interrupts();
                }
            }
            else
            {
                // Режим без ACK/NACK – простая обработка
                noInterrupts();
                inst->_mqtt_incoming_buffer.push(incoming_message);
                interrupts();
            }
        }
    }

    // Callback обработки события onRequest для I2C (передача сообщений)
    // Вызывается на стороне slave, когда master запрашивает данные
    static void static_on_request()
    {
        static MqttI2C* inst = MqttI2C::instance();
        static uint8_t msgSize = MqttMessageI2C::get_size_of();

        // На стороне slave, если используется ACK/NACK, сначала проверяем pending-ответ от master
        noInterrupts();
        if (inst->_use_ack_nack && inst->_slave_has_pending_response)
        {
            // Отправляем pending-ответ (ACK/NACK) от slave
            inst->_i2c->write(reinterpret_cast<uint8_t*>(&inst->_slave_pending_response), msgSize);
            inst->_slave_has_pending_response = false; // После отправки сбрасываем pending-ответ (ранее пояснял, просто флаг, что бы освободить для замещения в последующем)
            
            interrupts();
            return;
        }

        // Если нет pending-ответа, проверяем pending-сообщение для отправки slave->master
        if (inst->_use_ack_nack)
        {
            if (!inst->_slave_has_pending_outgoing)
            {
                // Берём новое сообщение из очереди, но не удаляем его окончательно – ждем ACK от master
                if (inst->_mqtt_outgoing_buffer.pop(inst->_slave_pending_outgoing)) {
                    inst->_slave_has_pending_outgoing = true;
                }
            }

            // Отправка сообщения, по запросу, на которое будем ждать ACK/NACK в static_on_receive() callback-е
            if (inst->_slave_has_pending_outgoing)
            {
                inst->_i2c->write(reinterpret_cast<uint8_t*>(&inst->_slave_pending_outgoing), msgSize);
            }
            else 
            {
                // Если нет сообщений в очереди, но почему-то отработал запрос на новые сообщения от master, то отправляем dummy/сообщение-пустышку
                // и заодно проверяем, что бы линия уведомления о новых сообщениях была LOW
                inst->_set_interrupt_signal(false);
                inst->_i2c->write(reinterpret_cast<uint8_t*>(MqttMessageI2C::get_dummy(inst->_address)), msgSize);
            }
        }
        else // Режим без ACK/NACK – просто извлекаем сообщение из очереди и отправляем master-у
        {
            MqttMessageI2C outgoing_message;
            if (inst->_mqtt_outgoing_buffer.pop(outgoing_message))
            {
                if (inst->_mqtt_outgoing_buffer.count() == 0)
                    inst->_set_interrupt_signal(false);
                else
                    outgoing_message.set_has_a_following_messages(true);

                inst->_i2c->write(reinterpret_cast<uint8_t*>(&outgoing_message), msgSize);
            }
            else
            {
                inst->_set_interrupt_signal(false);
                inst->_i2c->write(reinterpret_cast<uint8_t*>(MqttMessageI2C::get_dummy(inst->_address)), msgSize);
            }
        }
        interrupts();
    }

    // Статическая ISR-функция для рестарта I2C на слейве
    static void static_i2cRestartISR() {
        MqttI2C::instance()->_restart_i2c_slave();
    }

    // Установка состояния сигнала (на стороне слейва)
    void _set_interrupt_signal(bool state)
    {
        if (!_is_master)
            digitalWrite(_interrupt_pin, state);
    }

    // Получение состояния сигнала по пину
    bool _get_interrupt_signal(uint8_t address = 0x01)
    {
        if (!_is_master)
        {
            // Если slave - поулчить состояния своего пина информирвоания master-а
            return digitalRead(_interrupt_pin);
        }
        else if (address > 0x01 &&
                _master_side_subscriptions.find(address) != _master_side_subscriptions.end())
        {
            // Если master + адрес ведомого указан верно - получить состояние ноги уведомления от ведомого
            return digitalRead(_master_side_subscriptions[address].interrupt_pin);
        }
        else if (!_master_side_subscriptions.empty())
        {
            // Если master и без address - получить информацию, "вызывает" ли нас вообще хоть какой-нибудь из ведомых
            for (auto& sub_pair : _master_side_subscriptions)
                if (digitalRead(sub_pair.second.interrupt_pin))
                    return true;

            return false;
        }
        else
        {
            // Ошибка получения информации о состоянии сигнальной линии
            return false;
        }
    }

    // Метод рестарта I2C для мастера
    // Вызывается при обнаружении ошибок (когда I2C при вызове возвращает не 0, т.е. не I2C::OK)
    void restart_i2c_master()
    {
        if (_i2c && this->_is_master)
        {
            _i2c->end();
            _i2c->flush();
            _i2c->begin(_sda, _scl, _freq);
            delay(100);
        }
    }

    // Метод рестарта I2C для слейва
    // Вызывается при срабатывании прерывания на пине i2cRestartCallPin
    void _restart_i2c_slave()
    {
        if (_i2c && !this->_is_master)
        {
            _i2c->end();
            _i2c->flush();
            _i2c->begin(_address);
            _i2c->setClock(_freq);
            _i2c->onReceive(static_on_receive);
            _i2c->onRequest(static_on_request);
            delay(100);
        }
    }

    // Обработка подписчиков на стороне мастера – получение входящих сообщений от slave
    // Если режим ACK/NACK включён, после успешного чтения master сразу отправляет ACK на адрес slave,
    // чтобы slave мог удалить сообщение из очереди и не отсылать его повторно.
    void read_subscribers()
    {
        if (!_is_master)
            return;

        for (auto& sub_pair : _master_side_subscriptions)
        {
            uint8_t messagesReceived = 0;
            while (digitalRead(sub_pair.second.interrupt_pin) && messagesReceived < _max_messages_per_subscription)
            {
                int bytesReceived = _i2c->requestFrom(sub_pair.second.address, MqttMessageI2C::get_size_of());

                // Если по какой-то причине получено байт != размеру структуры сообщения MqttMessageI2C - сделать reset I2C
                if (bytesReceived != MqttMessageI2C::get_size_of())
                {
                    restart_i2c_master();

                    if(sub_pair.second.restart_pin != -1)
                    {
                        digitalWrite(sub_pair.second.restart_pin, HIGH);
                        delay(10);
                        digitalWrite(sub_pair.second.restart_pin, LOW);
                    }
                    break;
                }

                // Если всё ОК - продолжаем "читать" сообщения от slave-а 
                if (_i2c->available() >= MqttMessageI2C::get_size_of())
                {
                    MqttMessageI2C incoming;
                    _i2c->readBytes(reinterpret_cast<uint8_t*>(&incoming), MqttMessageI2C::get_size_of());
                    
                    // ACK/NACK (master, slave->master).
                    // Если режим включён, сразу после получения отправляем ACK на slave, чтобы он удалил сообщение.
                    if(_use_ack_nack)
                    {
                        MqttMessageI2C ackMsg;
                        ackMsg.command = MqttCommandI2C::ACK;
                        ackMsg.seq = incoming.seq;

                        // Обычно slave отправляет master-у сообщение со своим адресом, посему используем incoming.addr
                        // вместо sub_pair.second.address, т.е. экземпляр slave который опрашивается в for(...), т.к. адрес уже известен
                        _i2c->beginTransmission(incoming.addr); 
                        _i2c->write(reinterpret_cast<uint8_t*>(&ackMsg), MqttMessageI2C::get_size_of());
                        _i2c->endTransmission();
                    }
                    
                    noInterrupts();
                    _mqtt_incoming_buffer.push(incoming);
                    interrupts();
                    messagesReceived++;

                    // Если в самом сообщении от slave-а мы получили инфу, что сообщений
                    // у него больше нет (по стршему биту в command структуры), то выходим из цикла опроса slave
                    // по части опроса на получение сообщений
                    if (!incoming.get_has_a_following_messages())
                        break;
                }
                else
                {
                    // Если !i2c->availavble() (т.е. байт не получено по какой-то причине не получено), выходим из цикла опроса
                    break;
                }
            }
        }
    }

    // Обработка входящих (если быть точнее уже полученых) сообщений (у master и slave)
    // Здесь просто вызываются соответствующие обработчики для каждого полученного сообщения.
    void update_incoming()
    {
        if (_is_master)
        {
            MqttMessageI2C message;
            noInterrupts();
            while (_mqtt_incoming_buffer.pop(message))
            {
                interrupts();
                // Поиск подписчика по его адресу, для вызова зарегестрированных персонально
                // за данным slave-ом его handler-ов на поступающие сообщения
                auto it = _master_side_subscriptions.find(message.addr);

                if (it != _master_side_subscriptions.end())
                {
                    for (auto &handler_pair : it->second.command_handlers)
                    {
                        if (handler_pair.first == message.get_command()) {
                            handler_pair.second(message);
                        }
                    }
                }
                noInterrupts();
            }
            interrupts();
        }
        else
        {
            MqttMessageI2C message;
            noInterrupts();
            while (_mqtt_incoming_buffer.pop(message))
            {
                interrupts();
                for (auto &handler_pair : _slave_side_command_handlers)
                {
                    if (handler_pair.first == message.get_command()) {
                        handler_pair.second(message);
                    }
                }
                noInterrupts();
            }
            interrupts();
        }
    }

    // Обработка исходящих сообщений
    // Для мастера, если включён режим ACK/NACK, после отправки сообщения выполняется запрос для получения ACK/NACK от slave.
    // Если ACK получен с правильным seq – сообщение считается доставленным, иначе – возвращается в очередь для повторной отправки.
    // Для slave логика остаётся прежней – просто выставляется сигнал.
    void update_outgoing()
    {
        if (_is_master)
        {
            // Обработка сообщений master->slave с механизмом подтверждения ACK/NACK
            if (_use_ack_nack)
            {
                MqttMessageI2C next;

                noInterrupts();
                bool hasMsg = _mqtt_outgoing_buffer.pop(next);
                interrupts();

                if (hasMsg)
                {
                    next.seq = _master_seq_counter++; // Присваиваем последовательный номер

                    _i2c->beginTransmission(next.addr);
                    _i2c->write(reinterpret_cast<uint8_t*>(&next), MqttMessageI2C::get_size_of());
                    int result = _i2c->endTransmission();

                    // Если результат передачи по I2C != 0, т.е. != I2C::OK, то инициируем рестарт I2C с обеих сторон (master и slave)
                    if(result != 0)
                    {
                        restart_i2c_master();

                        // Рестарт I2C вызываем именно у того slave-а, с которым не удалось выполнить обмен данными
                        if (_master_side_subscriptions[next.addr].restart_pin != -1)
                        {
                            digitalWrite(_master_side_subscriptions[next.addr].restart_pin, HIGH);
                            delay(10);
                            digitalWrite(_master_side_subscriptions[next.addr].restart_pin, LOW);
                        }

                        // Т.к. произошла ошибка - возвращаем сообщение в очередь
                        noInterrupts();
                        _mqtt_outgoing_buffer.push(next, true);
                        interrupts();

                        // Выходим из метода update_outgoing();
                        return;
                    }

                    bool ackOk = false;

                    // Запрашиваем у slave подтверждение (ACK/NACK)
                    int ackBytes = _i2c->requestFrom(next.addr, MqttMessageI2C::get_size_of());

                    if (ackBytes >= MqttMessageI2C::get_size_of() &&
                        _i2c->available() >= MqttMessageI2C::get_size_of())
                    {
                        MqttMessageI2C ackMsg;
                        _i2c->readBytes(reinterpret_cast<uint8_t*>(&ackMsg), MqttMessageI2C::get_size_of());

                        if (ackMsg.get_command() == MqttCommandI2C::ACK &&
                            ackMsg.seq == next.seq)
                        {
                            ackOk = true;
                        }
                    }

                    if (!ackOk)
                    {
                        // Если не получен корректный ACK, возвращаем сообщение в очередь для повторной отправки
                        noInterrupts();
                        _mqtt_outgoing_buffer.push(next, true);
                        interrupts();
                    }
                }
            }
            else
            {
                // Режим без ACK/NACK – простая отправка
                MqttMessageI2C outgoing;

                noInterrupts();
                while (_mqtt_outgoing_buffer.pop(outgoing))
                {
                    interrupts();
                    _i2c->beginTransmission(outgoing.addr);
                    _i2c->write(reinterpret_cast<uint8_t*>(&outgoing), MqttMessageI2C::get_size_of());
                    int result = _i2c->endTransmission();

                    if(result != 0)
                    {
                        restart_i2c_master();

                        if (_master_side_subscriptions[outgoing.addr].restart_pin != -1)
                        {
                            digitalWrite(_master_side_subscriptions[outgoing.addr].restart_pin, HIGH);
                            delay(10);
                            digitalWrite(_master_side_subscriptions[outgoing.addr].restart_pin, LOW);
                        }
                    }
                    noInterrupts();
                }
                interrupts();
            }
        }
        else if (!this->_is_master && !_mqtt_outgoing_buffer.empty())
        {
            // Если это slave "собираается" отправить сообщения, то просто информируем master-а о наличии сообщений у slave-а
            _set_interrupt_signal(true);
        }
    }

public:
    static MqttI2C* instance()
    {
        static MqttI2C inst;
        return &inst;
    }

    // Инициализирует I2C соединение и возвращает указатель на экземпляр соединения 
    virtual I2C_CH* begin(
        uint8_t sda,
        uint8_t scl,
        unsigned int freq,
        bool is_master,
        uint8_t addr = 0x01,
        char interrupt_pin = -1,
        int i2c_restart_call_pin = -1
    ) override
    {
        _init_save_i2c_data(sda, scl, freq, is_master, addr, interrupt_pin, i2c_restart_call_pin);

        if (_is_master)
        {
#ifdef DEV_SSPS3_IS_MASTER
            this->_i2c = new I2C_CH(0);
            this->_i2c->begin(_sda, _scl, _freq);
#endif // !DEV_SSPS3_IS_MASTER
        }
        else
        {
#ifdef DEV_SSPS3_IS_MASTER
#else
            this->_i2c = new I2C_CH(_sda, _scl);
            this->_i2c->begin(_address);
            this->_i2c->setClock(_freq);

            this->_init_slave_i2c_interrupts();
#endif // !DEV_SSPS3_IS_MASTER
        }

        return this->_i2c;
    }
    
    // Не инициализирует соединение. Подразумевается, что пользователь уже настроил I2C извне.
    // Остальная информация передаётся с целью корректной работы рестарта I2C в случае "обрыва" соединения
    virtual I2C_CH* begin(
        I2C_CH* i2c_inited,
        uint8_t sda,
        uint8_t scl,
        unsigned int freq,
        bool is_master,
        uint8_t addr = 0x01,
        char interrupt_pin = -1,
        int i2c_restart_call_pin = -1
    ) override
    {
        _init_save_i2c_data(sda, scl, freq, is_master, addr, interrupt_pin, i2c_restart_call_pin);
        this->_i2c = i2c_inited;

        return this->_i2c;
    }

    // Метод добавления обработчика на поступающие сообщения от определенного slave-а
    // cmd - команда, которая должна быть обработана
    // handler - лямбда-метод с "MqttMessageI2C" аргументом, для обрботки полученного сообщения (полученное сообщение передаётся в лямбду)
    // address - адрес, для которого будет срабатывать данный обработчик поступающих сообщений
    virtual bool register_handler(
        uint8_t cmd,
        AfterReceiveHandler handler,
        uint8_t address = 0x01
    ) override
    {
        if (_is_master &&
            _master_side_subscriptions.find(address) != _master_side_subscriptions.end())
        {
            _master_side_subscriptions[address]
                .command_handlers
                .push_back({ cmd, handler });

            return true;
        }
        else if (!_is_master)
        {
            _slave_side_command_handlers
                .push_back({ cmd, handler });

            return true;
        }

        return false;
    }

    // Метод регистрации slave-а со стороны master-а, что бы "реагировать" на сигналы о поступающих сообщениях
    // и для последующего добавления обработчиков на добавленных ведомых
    // address - адрес slave-а
    // master_to_slave_signal_pin - пин у мастера, по которому slave сообщает о сообщениях
    // master_to_slave_restart_pin - пин у мастера, по которому тот сообщает slave-у о необходимости выполнить рестарт I2C
    virtual void subscribe_slave(
        uint8_t address,
        uint8_t master_to_slave_signal_pin,
        uint8_t master_to_slave_restart_pin = -1
    ) override
    {
        if (!_is_master)
            return;

        pinMode(master_to_slave_signal_pin, INPUT_PULLDOWN);

        if(master_to_slave_restart_pin != -1)
        {
            pinMode(master_to_slave_restart_pin, OUTPUT);
            digitalWrite(master_to_slave_restart_pin, LOW);
        }

        _master_side_subscriptions[address] =
            MqttSlaveSubscriber(
                address,
                master_to_slave_signal_pin,
                master_to_slave_restart_pin
            );
    }

    // Метод добвленеия сообщения с обеих сторон (master/slave) в очередь на отправку.
    // Отправка будет выполнена по факту вызова update(), например, в loop() проекта.
    // cmd - "MqttCommandI2C : uint8_t" команда сообщения
    // data - указатель на данные, которые собираемся отправлять (например некоторе значение типа {short})
    // lenght - размер отправляемых данных (например для некоторого значения типа short это будет {sizeof(short)})
    // dst - адрес назнчения сообщения. Если отправлет master, то обязательно указывать. Если отправляет slave, то не указываем
    // max_retries_spi - перегрузка от IMqtt интерфейса: не используется в реализации I2C, не указываем
    // ack_timeout_spi - перегрузка от IMqtt интерфейса: не используется в реализации I2C, не указываем
    virtual bool push_message(
        uint8_t cmd,
        const void* data,
        uint8_t length,
        uint8_t dst = 0x01, // dst - он же address для I2C реализации MQTT
        uint8_t max_retries_spi = 0,
        unsigned long ack_timeout_spi = 0
    ) override
    {
        if (_is_master &&
            dst > 0x01 &&
            _master_side_subscriptions.find(dst) != _master_side_subscriptions.end())
        {
            MqttMessageI2C new_message(cmd, dst);
            new_message.set_content(data, length);

            noInterrupts();
            bool result = _mqtt_outgoing_buffer.push(new_message);
            interrupts();
            
            return result;
        }
        else if (!_is_master)
        {
            MqttMessageI2C new_message(cmd, this->_address);
            new_message.set_content(data, length);

            noInterrupts();
            bool result = _mqtt_outgoing_buffer.push(new_message);
            interrupts();

            _set_interrupt_signal(true);
            return result;
        }
        
        return false;
    }

    virtual void update() override
    {
        read_subscribers();
        update_incoming();
        update_outgoing();
    }
};

#endif
