#pragma once
#ifndef TASK_EXECUTOR_H
#define TASK_EXECUTOR_H

#include <Arduino.h>
#include "../Data/t_datetime.h"
#include "../ExternalEnvironment/io_physical_monitor.h"
#include "./Provider/task_model_to_memory_provider.h"

#pragma pack(push, 1)
struct TaskExecutor
{
private:
    // Получаем указатель на TaskInstruction из XVarArray<TaskInstruction, SIZE>
    // за счёт вызова get_task_instruction() метода из синглтон класса-проводника,
    // до хранилища XVar_*** переменных к которым получаем доступ через передачу
    // handler-а извне во время инициализации x_var_data_center.h 
    TaskInstruction* _instruction(short index = -1) {
        return TaskToMem->get_task_instruction(index < 0 ? current_instruction_index : index);
    }

    // Будет вызываться как раз в секунду, так и по факту изменений
    void _instruction_save(short index = -1) {
        TaskToMem->save_task_instruction_changes(index < 0 ? current_instruction_index : index);
    }

    // По логике будет вызываться раз в секунду, т.к. изменения лишь раз в секунду
    void _executor_save() {
        TaskToMem->save_task_executor_changes();
    }

public:
    char name[20];
    unsigned char current_instruction_index;    // индекс текущей инструкции (начинается с 0)
    unsigned char total_instructions_count;     // общее количество инструкций (массив находится вне класса)
    unsigned char states;                       // битовое поле для состояний TaskExecutor-исполнителя
    unsigned short max_idle_on_pause_ss;        // максимальное время простоя в паузе (сек)
    unsigned long critical_idle_ss;             // накопленное время простоя (сек)
    unsigned int critical_idle_count;           // счётчик срабатываний простоя
    XDateTime when_started_dt;                   // время старта работы
    XDateTime last_iteration_dt;                 // время последней итерации controller()
    short temperature_condition_offset_C;       // допуск по температуре (например, ±5°C)

    enum TaskExecutorError
    {
        ErrorLongIdleByUserPause,
        ErrorLongIdleByNo380v,
        ErrorLongIdleByNoWaterInJacket,
        ErrorMixerCrash
    };

    // Битовые маски для состояний исполнителя
    enum TaskExecutorStateBits
    {
        IS_RUNNING                = 1 << 0,
        IS_ON_PAUSE               = 1 << 1,
        IS_ON_PAUSE_BY_USER       = 1 << 2,
        IS_ON_PAUSE_BY_WATER_JACKET_DRAIN = 1 << 3,
        IS_ON_PAUSE_BY_NO_380V    = 1 << 4,
        IS_COMPLETED              = 1 << 5,
        IS_COMPLETED_WITH_ERROR   = 1 << 6
    };

    enum TaskCheckpointResult
    {
        POINT_RETURN_FALSE,
        POINT_RETURN_TRUE,
        POINT_CONTINUE
    };

    TaskExecutor() :
        current_instruction_index(0),
        total_instructions_count(0),
        states(0),
        max_idle_on_pause_ss(1800),
        critical_idle_ss(0),
        critical_idle_count(0),
        temperature_condition_offset_C(3)
    {
        name[0] = '\0';
    }
    
    // Инициализация исполнителя. Выполняется только если исполнитель завершён.
    void init(const char* _name,
              unsigned char _total_instructions_count,
              unsigned short _max_idle_on_pause_ss,
              const XDateTime& _when_started_dt,
              short _temperature_condition_offset_C)
    {
        if (!(states & IS_COMPLETED || states & IS_COMPLETED_WITH_ERROR))
            return;
            
        strncpy(name, _name, sizeof(name) - 1);
        name[sizeof(name) - 1]          = '\0';
        
        current_instruction_index       = 0;
        total_instructions_count        = _total_instructions_count;
        max_idle_on_pause_ss            = _max_idle_on_pause_ss;
        critical_idle_ss                = 0;
        critical_idle_count             = 0;
        when_started_dt                 = _when_started_dt;
        last_iteration_dt               = _when_started_dt;
        temperature_condition_offset_C  = _temperature_condition_offset_C;
        
        // Запускаем исполнение
        states = IS_RUNNING;

        // Выполняем подготовку controller(...) метода через сброс его статических таймеров и флагов
        controller(_when_started_dt, true, true); // последний true и есть флаг сброса
    }

    // Геттеры и сеттеры для флагов состояния
    bool is_running() const { return states & IS_RUNNING; }
    void set_is_running(bool value) {
        if (value) states |= IS_RUNNING; else states &= ~IS_RUNNING;
    }
    
    bool is_on_pause() const { return states & IS_ON_PAUSE; }
    void set_is_on_pause(bool value) {
        if (value) states |= IS_ON_PAUSE; else states &= ~IS_ON_PAUSE;
    }
    
    bool is_on_pause_by_user() const { return states & IS_ON_PAUSE_BY_USER; }
    void set_is_on_pause_by_user(bool value) {
        if (value) states |= IS_ON_PAUSE_BY_USER; else states &= ~IS_ON_PAUSE_BY_USER;
    }
    
    bool is_on_pause_by_water_jacket_drain() const { return states & IS_ON_PAUSE_BY_WATER_JACKET_DRAIN; }
    void set_is_on_pause_by_water_jacket_drain(bool value) {
        if (value) states |= IS_ON_PAUSE_BY_WATER_JACKET_DRAIN; else states &= ~IS_ON_PAUSE_BY_WATER_JACKET_DRAIN;
    }
    
    bool is_on_pause_by_no_380v() const { return states & IS_ON_PAUSE_BY_NO_380V; }
    void set_is_on_pause_by_no_380v(bool value) {
        if (value) states |= IS_ON_PAUSE_BY_NO_380V; else states &= ~IS_ON_PAUSE_BY_NO_380V;
    }
    
    bool is_completed() const { return states & IS_COMPLETED; }
    void set_is_completed(bool value) {
        if (value) states |= IS_COMPLETED; else states &= ~IS_COMPLETED;
    }
    
    bool is_completed_with_error() const { return states & IS_COMPLETED_WITH_ERROR; }
    void set_is_completed_with_error(bool value) {
        if (value) states |= IS_COMPLETED_WITH_ERROR; else states &= ~IS_COMPLETED_WITH_ERROR;
    }
    
    // Метод суммирования времени in_process_ss для инструкций от 0 до текущей
    unsigned long total_in_process_ss()
    {
        unsigned long total = 0;

        for (unsigned char i = 0; i <= current_instruction_index; i++)
            total += _instruction(i)->get_in_process_ss();

        return total;
    }
    
    const char* total_in_process_ss_to_string()
    {
        static char buffer[16];
        unsigned long total_sec = total_in_process_ss();
        unsigned long hours = total_sec / 3600;
        unsigned long minutes = (total_sec % 3600) / 60;
        unsigned long seconds = total_sec % 60;
        
        if (hours < 100)
            snprintf(buffer, sizeof(buffer), "%02lu:%02lu:%02lu", hours, minutes, seconds);
        else
            snprintf(buffer, sizeof(buffer), "%03lu:%02lu:%02lu", hours, minutes, seconds);
        return buffer;
    }
    
    // ВАЖНО: все методы ниже оказывают влияние на ключевые переменные, отвечающие
    // как за работоспособность данного класса, так и влияющие на поведение 
    // исполнения "Инструкций", а отсюда физической периферией соответсвенно.
    // Данные должны всегда быть актуальными не только в ОЗУ master-а,
    // но и сразу же сохраняться во внешнюю память. Необходимо для быстрого восстановления
    // после экстренных ситуаций по типу потери питания у оборудования.
    // Отсюда уточнение - перепроверить, что бы методах ниже присутствовали
    // обработчики сохранений "_instruction_save()" и/или "_executor_save()".
    // Если по какой-то причине метод не нуждается в использовании методов
    // перезаписи данных во внешней флешке на актуальные - указать на это комментарием.

    // Методы для увеличения in_process_ss и уменьшения duration_aim_left_ss для текущей инструкции
    void instruction_in_process_ss_inc()
    {
        _instruction()->increase_in_process_ss();
        _instruction_save();
    }

    void instruction_duration_aim_left_ss_dec()
    {
        _instruction()->decrease_duration_aim_left_ss();
        _instruction_save();
    }
    
    // Методы переключения паузы
    void pause_by_user_plc_button()
    {
        set_is_on_pause_by_user(!is_on_pause_by_user());

        if (!is_on_pause_by_user() &&
            !is_on_pause_by_water_jacket_drain() &&
            !is_on_pause_by_no_380v())
            set_is_on_pause(false);
        else
            set_is_on_pause(true);

        _executor_save();
    }

    void pause_by_water_jacket_drain(bool flag)
    {
        set_is_on_pause_by_water_jacket_drain(flag);

        if(flag)
            set_is_on_pause(true);
        else if (!is_on_pause_by_user() && !is_on_pause_by_no_380v())
            set_is_on_pause(false);

        _executor_save();
    }

    void pause_by_no_380v(bool flag)
    {
        set_is_on_pause_by_no_380v(flag);

        if(flag)
            set_is_on_pause(true);
        else if (!is_on_pause_by_user() && !is_on_pause_by_water_jacket_drain())
            set_is_on_pause(false);

        _executor_save();
    }
    
    // Подтверждение завершения (без ошибки)
    void confirm_as_completed()
    {
        _instruction()->set_is_completed(true);
        _instruction()->set_is_in_process(false);
        _instruction_save();

        set_is_running(false);
        set_is_completed(true);
        set_is_completed_with_error(false);
        _executor_save();

        // Выключение физических устройств:
        IOMonitor->set_output_state(DOUT::HEATERS_RELAY, false, true);
        IOMonitor->set_output_state(DOUT::WJACKET_RELAY, false, true);
        IOMonitor->set_motor_speed(0, false);

        // Обрабатываем успешное завершение, например выводим на экран и сохраняем в логи (во внешнюю FRAM).
        // Можно добавить синглтон или модифицировать текущий синглтон провайдер,
        // куда можно добавить метод-обработчик передаваемых в него enum::TaskExecutorError ошибок
        //------------------------------
        // Добавить обработку завершения
        //------------------------------
        /*********************************************************************************************************************/
    }

    // Завершение с ошибкой
    void confirm_as_completed_with_error(TaskExecutorError error)
    {
        _instruction()->set_is_cancelled(true);
        _instruction()->set_is_in_process(false);
        _instruction_save();
        
        set_is_running(false);
        set_is_completed(true);
        set_is_completed_with_error(true);
        _executor_save();
        
        // Выключение физических устройств:
        IOMonitor->set_output_state(DOUT::HEATERS_RELAY, false, true);
        IOMonitor->set_output_state(DOUT::WJACKET_RELAY, false, true);
        IOMonitor->set_motor_speed(0, false);

        // Обрабатываем ошибку, например выводим на экран и сохраняем в логи (во внешнюю FRAM).
        // Можно добавить синглтон или модифицировать текущий синглтон провайдер,
        // куда можно добавить метод-обработчик передаваемых в него enum::TaskExecutorError ошибок
        //--------------------------
        // Добавить обработку ошибок
        //--------------------------
        /*********************************************************************************************************************/
    }
    
    // Метод перехода к следующей инструкции (при подтверждении пользователем)
    void accept_instruction_as_completed_by_user()
    {
        if (current_instruction_index < total_instructions_count - 1)
        {
            _instruction()->set_is_completed(true);
            _instruction()->set_is_in_process(false);
            _instruction_save();
            ++current_instruction_index;
        }
        else
            confirm_as_completed();
    }
    
    // Изменение максимального времени простоя в паузе
    void any_pause_max_idle_duration_ss(unsigned long idle_limit_ss)
    {
        max_idle_on_pause_ss = idle_limit_ss;
        _executor_save();
    }
    
    // Установка допуска температуры для подтверждения перехода на следующий этап (например, ±5°C)
    void set_temperature_condition_offset(short offset)
    {
        temperature_condition_offset_C = offset;
        _executor_save();
    }

    // Вызываемый извне, например где-нибудь в loop(){...}, обработчик,
    // который можно нагрузить доп логикой, когда исполняется основная логика
    // метода controller(...), что можно выявить относительно возвращаемого им true/false
    bool update(XDateTime rt_dt, bool execute_immediately = false)
    {
        TaskCheckpointResult _execute_result = controller(rt_dt, execute_immediately, false);

        // Вариант обработки состояния вызова исполнения
        // switch(_execute_result)
        // {
        //     case POINT_RETURN_FALSE:    {}; break;
        //     case POINT_RETURN_TRUE:     {}; break;
        //     case POINT_CONTINUE:        {}; break;
        // }

        return _execute_result != POINT_RETURN_FALSE ? true : false;
    }

    // Полностью реализованный controller(), исполняющий свою логику каждые 250 мс (или немедленно, если execute_immediately==true)
    // Возвращаемый bool - был ли исполнен обработчик по execute_immediately или через 250мс.
    // По факту возвращаемый bool будет == false, только во время простоя из-за таймера в методе
    // if (!execute_immediately && (current_ms - last_250ms_ms < 250)) {
    // return false;
    // }
    TaskCheckpointResult controller(XDateTime rt_dt, bool execute_immediately = false, bool new_task_started = false)
    {
        unsigned long current_ms = millis();
        // Если исполнитель не запущен – раз в 10 сек отключаем физические устройства
        static unsigned long last_disable_ms = 0;

        // Используем static переменные для таймеров внутри controller()
        static bool any_pause_timer_is_active               = false;
        static bool water_jacket_timer_is_active            = false;
        static unsigned long last_1000ms_aim_left_ms        = 0;
        static unsigned long last_1000ms_in_proc_ms         = 0;
        static unsigned long last_250ms_ms                  = 0;
        static unsigned long any_pause_timer_start_ms       = 0;
        static unsigned long water_jacket_timer_start_ms    = 0;

        TaskCheckpointResult _checkpoint_result = POINT_CONTINUE;

        _checkpoint_result = _checkpoint_reset_timers_if_new_task_started(
            new_task_started,
            any_pause_timer_is_active,
            water_jacket_timer_is_active,
            current_ms,
            last_1000ms_aim_left_ms,
            last_1000ms_in_proc_ms,
            last_250ms_ms,
            any_pause_timer_start_ms,
            water_jacket_timer_start_ms
        );
        if (_checkpoint_result != POINT_CONTINUE) return _checkpoint_result;

        _checkpoint_result = _checkpoint_to_do_on_task_of_10sec(current_ms, last_disable_ms, execute_immediately);
        if (_checkpoint_result != POINT_CONTINUE) return _checkpoint_result;

        _checkpoint_result = _checkpoint_execute_instruction(execute_immediately, current_ms, last_250ms_ms);
        if (_checkpoint_result != POINT_CONTINUE) return _checkpoint_result;

        // Если один датчик даёт температуру, сохраняем его значение.
        // Если два датчика дают температуру, то ищем среднее арифметическое значение
        // между ((темп. продукта + темп. воды в рубашке) / 2 датчика - 1 градус) для чуть более агресивного нагрева
        float temperature_C_product     = IOMonitor->get_temperature_C_product();
        float temperature_C_wjacket     = IOMonitor->get_temperature_C_wjacket();
        
        bool is_product_C_below_zero    = temperature_C_product < 0.f;
        bool is_prodcut_C_sens_error    = temperature_C_product < -40.f || temperature_C_product > 105.f;
        bool is_wjacket_C_sens_error    = temperature_C_wjacket < -40.f || temperature_C_wjacket > 105.f;
        
        float temperature_C_current = is_wjacket_C_sens_error
            ? temperature_C_product
            : ((temperature_C_product + temperature_C_wjacket) / 2.f) - 1.f;

        //------------------------------------------------------
        // Тут можно добавить чекпоинт на проверку, не является
        // ли температура совсем уже неправдоподобной и если да,
        // то завершать работу. if(is_prodcut_C_sens_error)
        //------------------------------------------------------

        _checkpoint_result = _checkpoint_detect_task_idle_by_long_power_off(rt_dt);
        if (_checkpoint_result != POINT_CONTINUE) return _checkpoint_result;

        _checkpoint_result = _checkpoint_does_mixer_crush();
        if (_checkpoint_result != POINT_CONTINUE) return _checkpoint_result;
        
        _checkpoint_result = _checkpoint_on_pause_handler(any_pause_timer_is_active, any_pause_timer_start_ms, current_ms);
        if (_checkpoint_result != POINT_CONTINUE) return _checkpoint_result;

        _checkpoint_result = _checkpoint_have_380V_power();
        if (_checkpoint_result != POINT_CONTINUE) return _checkpoint_result;

        // Достаём нашу инструкцию по индексу current_instruction_index из _instruction() и сохраняем в _curr_instruction
        TaskInstruction* _curr_instruction = _instruction();
        
        // Всем инстуркциям предшествует real-time/"фоновая" проверка на "протечку" водянйо рубашки
        _checkpoint_result = _checkpoint_water_jacket_draining_handler(
            _curr_instruction,
            water_jacket_timer_is_active,
            water_jacket_timer_start_ms,
            current_ms
        );
        if (_checkpoint_result != POINT_CONTINUE) return _checkpoint_result;
        
        //--------------------------------------------------------------------
        // Закончили с проверками.
        // Приступаем непосредственно к работе с инструкциями
        //--------------------------------------------------------------------
        
        // Если инструкция находится в очереди, переводим её в режим исполнения
        if (_curr_instruction->get_is_in_queue())
        {
            last_1000ms_aim_left_ms = 0;
            _curr_instruction->set_is_in_queue(false);
            _curr_instruction->set_is_in_process(true);
            _instruction_save();
        }

        // Обновляем таймеры и сохраняем состояние исполнителя и инструкции раз в 1000 мс
        if (current_ms - last_1000ms_in_proc_ms >= 1000)
        {
            last_1000ms_in_proc_ms = current_ms;
            _executor_save();
            instruction_in_process_ss_inc();
        }
        
        // Логика для этапа набора воды
        if (_curr_instruction->get_is_water_intake_step())
        {
            IOMonitor->set_output_state(DOUT::HEATERS_RELAY, false);
            IOMonitor->set_motor_speed(
                is_product_C_below_zero ? 0 : _curr_instruction->rot_per_min,
                _curr_instruction->get_is_fast_mixer_mode_step()
            );

            // Если вода есть - закрываем клапан набора воды в рубашку
            // и переходим ко следующей инструкции, либо заливаем дальше
            if (IOMonitor->get_input_state(DIN::WATER_JACKET_SIGNAL))
            {
                IOMonitor->set_output_state(DOUT::WJACKET_RELAY, false);
                
                accept_instruction_as_completed_by_user();
                return POINT_RETURN_TRUE;
            }
            else
            {
                // Если воды нет – открываем клапан набора воды в рубашку
                IOMonitor->set_output_state(DOUT::WJACKET_RELAY, true);

                return POINT_RETURN_TRUE;
            }
        }
        
        //--------------------------------------------------------------------
        // Ключевой блок - контроль температуры, охлаждения, оборотов,
        // а так же ожидания подтверждения пользователя по необходимости,
        // когда единственный "особенный" is_water_intake_step этап пройден.
        //--------------------------------------------------------------------

        // Обработка температуры и таймеров для инструкции
        // Если установлен ONLY_UNTIL_CONDITION_MET – переход по достижении температуры (с допуском)
        bool until_condition_met  = _curr_instruction->get_is_only_until_condition_met();
        short target_temp_C = _curr_instruction->temperature_C;

        if (!until_condition_met && abs(temperature_C_current - target_temp_C) <= temperature_condition_offset_C)
        {
            // Обновляем таймеры для самих instruction, если не until_condition_met 
            if (current_ms - last_1000ms_aim_left_ms >= 1000)
            {
                if (last_1000ms_aim_left_ms != 0)
                    instruction_duration_aim_left_ss_dec();
            
                last_1000ms_aim_left_ms = current_ms;
            }
        }

        // Если until_condition_met  и температура достигнута           – завершаем инструкцию
        // Есил !until_condition_met и таймер duration_aim_left_ss <= 0 – завершаем инструкцию
        if ((until_condition_met  && abs(temperature_C_current - target_temp_C) <= temperature_condition_offset_C) ||
            (!until_condition_met && _curr_instruction->duration_aim_left_ss    <= 0))
        {
            // Поставим флаг как get_is_completed == 1 через set_is_completed(true)
            // оставив флаг get_is_in_process тоже как == 1.
            // get_is_in_process будет сброшен по факту окончателнього завершения
            // инструкции после вызова accept_instruction_as_completed_by_user(), в котором:
            // 1) get_is_in_process = 0
            // 2) ++current_instruction_index
            _curr_instruction->set_is_completed(true);
            
            // Если по окончанию выполнения инструкции флаг get_is_await_user_accept_when_completed
            // указывает, что нам необходимо ждать кнопку подтверждения пользоватея - ожидаем
            if (_curr_instruction->get_is_await_user_accept_when_completed())
            {
                IOMonitor->set_output_state(DOUT::HEATERS_RELAY, temperature_C_current < target_temp_C);
                IOMonitor->set_output_state(
                    DOUT::WJACKET_RELAY,
                    temperature_C_current > target_temp_C &&
                    (_curr_instruction->get_is_active_cooling() ||
                    _curr_instruction->get_is_last_step())
                );
                
                if (_curr_instruction->get_is_dont_rotate_on_user_await())
                    IOMonitor->set_motor_speed(0, false);
                else
                    IOMonitor->set_motor_speed(
                        is_product_C_below_zero ? 0 : _curr_instruction->rot_per_min,
                        _curr_instruction->get_is_fast_mixer_mode_step()
                    );

                // Ждём подтверждения от пользователя (метод accept_instruction_as_completed_by_user() вызывается извне)
                //--------------------------------
                // Добавить логику ожидания user-а
                //--------------------------------
                /*********************************************************************************************************************/
            }
            else // в ином случае просто завершаем этап
            {
                accept_instruction_as_completed_by_user();
            }

            return POINT_RETURN_TRUE;
        }
        else // Если не выполнены усовия, что бы считать данный этап завершенным, то выполняем следующий кейс 
        {
            // Устанавливаем скорость, заданную в инструкции
            IOMonitor->set_motor_speed(
                is_product_C_below_zero ? 0 : _curr_instruction->rot_per_min,
                _curr_instruction->get_is_fast_mixer_mode_step()
            );
            
            // Управление: если температура ниже – нагреваем
            if (temperature_C_current < target_temp_C) 
            {
                // Включаем нагрев
                IOMonitor->set_output_state(DOUT::HEATERS_RELAY, true);
                IOMonitor->set_output_state(DOUT::WJACKET_RELAY, false);
            }
            else // Управление: если температура выше – охлаждаем (при флаге активного охлаждения)
            if (temperature_C_current > target_temp_C) 
            {
                // При охлаждении           – отключаем нагрев
                // При активном охлаждении  – включаем клапан набора воды
                IOMonitor->set_output_state(DOUT::HEATERS_RELAY, false);
                IOMonitor->set_output_state(
                    DOUT::WJACKET_RELAY,
                    _curr_instruction->get_is_active_cooling() ||
                    _curr_instruction->get_is_last_step()
                );                
            }
            else // температура точно равна требуемой в инструкции - выкючаем всё
            {
                IOMonitor->set_output_state(DOUT::HEATERS_RELAY, false);
                IOMonitor->set_output_state(DOUT::WJACKET_RELAY, false);
            }

            return POINT_RETURN_TRUE;
        }
    }

private:
    TaskCheckpointResult _checkpoint_reset_timers_if_new_task_started(
        bool _new_task_started,
        bool& _any_pause_timer_is_active,
        bool& _water_jacket_timer_is_active,
        unsigned long& _current_ms,
        unsigned long& _last_1000ms_aim_left_ms,
        unsigned long& _last_1000ms_in_proc_ms,
        unsigned long& _last_250ms_ms,
        unsigned long& _any_pause_timer_start_ms,
        unsigned long& _water_jacket_timer_start_ms
    )
    {
        if (_new_task_started)
        {
            _last_1000ms_aim_left_ms = 0;
            
            _last_250ms_ms
                = _last_1000ms_in_proc_ms
                = _current_ms;

            _any_pause_timer_start_ms
                = _water_jacket_timer_start_ms
                = 0;

            _any_pause_timer_is_active
                = _water_jacket_timer_is_active
                = false;

            IOMonitor->set_output_states_all(false);

            return POINT_RETURN_FALSE;
        }
        return POINT_CONTINUE;
    }

    TaskCheckpointResult _checkpoint_to_do_on_task_of_10sec(unsigned long& _current_ms, unsigned long& _last_disable_ms, bool _execute_immediately)
    {
        if (!is_running())
        {
            if (_current_ms - _last_disable_ms >= 10000 || _execute_immediately)
            {
                // Подача команды на отключение каждые 10 сек на всякий случай:
                IOMonitor->set_output_state(DOUT::HEATERS_RELAY, false);
                IOMonitor->set_output_state(DOUT::WJACKET_RELAY, false);
                IOMonitor->set_motor_speed(0, false);

                _last_disable_ms = _current_ms;                
                return POINT_RETURN_TRUE;
            }
            else 
                return POINT_RETURN_FALSE;
        }

        return POINT_CONTINUE;
    }

    TaskCheckpointResult _checkpoint_detect_task_idle_by_long_power_off(XDateTime& rt_dt)
    {
        // Вычисляем разницу между rt_dt и последней итерацией (в секундах)
        // если оборудование включено, то метод update будет вызван раз 400 в 1000мс
        // однако из-за блока last_250ms_ms до данного кейса дойдём минимум 4 раза.
        // Этого достаточно, что бы не возникало пробем для контроля и исполнения
        // порядка инструкций. Однако если сеть была выкючена, заетм включии оборудование
        // то при вызове controller() будет обнаружено время простоя и обработано в 2 важных сценария:
        // если > 5 и < 1800 (30мин), то сохраняем инфу;
        // если > 1800, то выеротно за такое длительное врем простояя молоко или иной продукт
        // уже испортились и мы не несём ответствености за это, посему завершаем программу

        XTimeSpan diff = rt_dt - last_iteration_dt;
        last_iteration_dt = rt_dt; // сохранили настоящее текущее время относительно DS3231
        int diff_sec = diff.total_seconds_abs();
        if (diff_sec < 5)
        {
            // Всё ок, вдруг затуп произошёл и передача больших данных
            // поставила ПЛК в небольшой простой, забрав немного процессорного
            // времени на себя
        }
        else if (diff_sec >= 5 && diff_sec <= 1800)
        {
            // суммируем время киртического простоя и количество таких простоев
            critical_idle_ss += diff_sec;
            ++critical_idle_count;
        }
        else if (diff_sec >= 1800)
        {
            // продукт вероятно испорчен, завершаем работу
            critical_idle_ss += diff_sec;
            ++critical_idle_count;
             
            confirm_as_completed_with_error(ErrorLongIdleByUserPause);
            return POINT_RETURN_TRUE;
        }

        return POINT_CONTINUE;
    }

    TaskCheckpointResult _checkpoint_execute_instruction(bool& _execute_immediately, unsigned long& _current_ms, unsigned long& _last_250ms_ms)
    {
        // Если не execute_immediately – обновляем логику раз в 250 мс
        if (!_execute_immediately && (_current_ms - _last_250ms_ms < 250))
            return POINT_RETURN_FALSE;
        
        _last_250ms_ms = _current_ms;
        return POINT_CONTINUE;
    }

    TaskCheckpointResult _checkpoint_does_mixer_crush()
    {
        // Проверка аварии мотора
        if(IOMonitor->get_input_state(DIN::MIXER_ERROR_SIGNAL))
        {
            confirm_as_completed_with_error(ErrorMixerCrash);
            return POINT_RETURN_TRUE;
        }

        return POINT_CONTINUE;
    }

    TaskCheckpointResult _checkpoint_on_pause_handler(
        bool& _any_pause_timer_is_active,
        unsigned long& _any_pause_timer_start_ms,
        unsigned long& _current_ms
    )
    {
        // Если включены какие-либо паузы – проверяем время простоя
        // и выходим из метода, пока не будут разрешены проблемы.
        // При превышении дительности простоя относительно max_idle_on_pause_ss
        // смотрим, на какой именно паузе мы находились и в зависимости от
        // неё отправляем error методу max_idle_on_pause_ss.
        if(is_on_pause())
        {
            if (!_any_pause_timer_is_active)
            {
                _any_pause_timer_is_active = true;
                _any_pause_timer_start_ms = _current_ms;
            }

            if ((_current_ms - _any_pause_timer_start_ms) / 1000 >= max_idle_on_pause_ss)
            {
                if(is_on_pause_by_user())
                    confirm_as_completed_with_error(ErrorLongIdleByUserPause);
                else
                if(is_on_pause_by_no_380v())
                    confirm_as_completed_with_error(ErrorLongIdleByNo380v);
                else
                if(is_on_pause_by_water_jacket_drain())
                    confirm_as_completed_with_error(ErrorLongIdleByNoWaterInJacket);

                return POINT_RETURN_TRUE;
            }

            pause_by_water_jacket_drain(!IOMonitor->get_input_state(DIN::WATER_JACKET_SIGNAL));
            pause_by_no_380v(!IOMonitor->get_input_state(DIN::V380_SIGNAL));

            // если ошибки исправлены (два if case-а выше), но всё-равно стоим на паузе
            // значит, вероятно, мы ещё на последней вероятной паузе - по кнопке пользователя.
            // тогда продожаем прерывать испонение инструкции
            if (is_on_pause())
            {
                // Если в паузе – останавливаем таймеры и физические устройства
                // кроме кейса, когда have_water_in_water_jacket == false, ибо в таком случае как раз нужен набор воды.
                // (физ. команды ниже – примеры)

                IOMonitor->set_output_state(DOUT::HEATERS_RELAY, false);
                IOMonitor->set_output_state(DOUT::WJACKET_RELAY, is_on_pause_by_water_jacket_drain());
                IOMonitor->set_motor_speed(0, false);

                return POINT_RETURN_TRUE;
            }
        }
        else if (_any_pause_timer_is_active)
        {
            _any_pause_timer_is_active = false;
            _any_pause_timer_start_ms = 0;
        }

        return POINT_CONTINUE;
    }

    TaskCheckpointResult _checkpoint_have_380V_power()
    {
        // Проверка 380V
        if(!IOMonitor->get_input_state(DIN::V380_SIGNAL))
        {
            pause_by_no_380v(true);
            return POINT_RETURN_TRUE;
        }

        return POINT_CONTINUE;
    }

    TaskCheckpointResult _checkpoint_water_jacket_draining_handler(
        TaskInstruction* _curr_instruction,
        bool& _water_jacket_timer_is_active,
        unsigned long& _water_jacket_timer_start_ms,
        unsigned long& _current_ms
    )
    {
        // Подразумевается, что дойдя до текущей точки - мы не находимсяя на паузе и/или программа
        // не завершена, а значит дальше проверяем в реальном времени и доливаем воду в рубашку,
        // в ином сучае ставим программу как раз на паузу, в которой тоже будет доливаться вода в рубашку.
        // Итого: если воды нет И ЭТО НЕ ЭТАП НАБОРА ВОДЫ, то включаем клапан и запускаем 30-секундный таймер
        if(!_curr_instruction->get_is_water_intake_step())
        {
            if (!IOMonitor->get_input_state(DIN::WATER_JACKET_SIGNAL))
            {
                if (!_water_jacket_timer_is_active)
                {
                    _water_jacket_timer_is_active = true;
                    _water_jacket_timer_start_ms = _current_ms;
                }

                // Включаем клапан для набора воды
                IOMonitor->set_output_state(DOUT::WJACKET_RELAY, true);

                if(_current_ms - _water_jacket_timer_start_ms >= 30000)
                {
                    // Если воды так и не появилось – переводим в паузу по сливу рубашки
                    pause_by_water_jacket_drain(true);
                    return POINT_RETURN_TRUE;
                }
            }
            else if (_water_jacket_timer_is_active)
            {
                _water_jacket_timer_is_active = false;
                _water_jacket_timer_start_ms = 0;

                IOMonitor->set_output_state(DOUT::WJACKET_RELAY, false);
            }
        }

        return POINT_CONTINUE;
    }

public:
    bool operator==(const TaskExecutor& other) { return false; }
};
#pragma pack(pop)

#endif // !TASK_EXECUTOR_H