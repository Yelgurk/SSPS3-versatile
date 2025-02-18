#pragma once
#ifndef T_TASK_EXECUTOR_H
#define T_TASK_EXECUTOR_H

#include <Arduino.h>
#include "./t_datetime.h"
#include "./t_task_instruction.h"

#pragma pack(push, 1)
struct TaskExecutor {
    char name[20];
    unsigned char current_instruction_index;   // индекс текущей инструкции (начинается с 0)
    unsigned char total_instructions_count;      // общее количество инструкций (массив находится вне класса)
    unsigned char states;                        // битовое поле для состояний исполнителя
    unsigned short max_idle_on_pause_ss;         // максимальное время простоя в паузе (сек)
    unsigned long critical_idle_ss;              // накопленное время простоя (сек)
    unsigned int critical_idle_count;            // счётчик срабатываний простоя
    DateTime when_started_dt;                     // время старта работы
    DateTime last_iteration_dt;                   // время последней итерации update()
    short temperature_condition_offset_C;        // допуск по температуре (например, ±5°C)

    enum TaskExecutorError {
        ErrorLongIdleByUserPause,
        ErrorLongIdleByNo380v,
        ErrorLongIdleByNoWaterInJacket,
        ErrorMixerCrash
    };

    // Битовые маски для состояний исполнителя
    enum ExecutorStateBits {
        IS_RUNNING                = 1 << 0,
        IS_ON_PAUSE               = 1 << 1,
        IS_ON_PAUSE_BY_USER       = 1 << 2,
        IS_ON_PAUSE_BY_WATER_JACKET_DRAIN = 1 << 3,
        IS_ON_PAUSE_BY_NO_380V    = 1 << 4,
        IS_COMPLETED              = 1 << 5,
        IS_COMPLETED_WITH_ERROR   = 1 << 6
    };

    TaskExecutor() : current_instruction_index(0), total_instructions_count(0), states(0),
                 max_idle_on_pause_ss(0), critical_idle_ss(0), critical_idle_count(0),
                 temperature_condition_offset_C(5)
    {
        name[0] = '\0';
    }
    
    // Инициализация исполнителя. Выполняется только если исполнитель завершён.
    void init(const char* _name,
              unsigned char _total_instructions_count,
              unsigned short _max_idle_on_pause_ss,
              const DateTime& _when_started_dt,
              const DateTime& _last_iteration_dt,
              short _temperature_condition_offset_C)
    {
        if (!(states & IS_COMPLETED || states & IS_COMPLETED_WITH_ERROR))
            return;
            
        strncpy(name, _name, sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';
        
        current_instruction_index = 0;
        total_instructions_count = _total_instructions_count;
        max_idle_on_pause_ss = _max_idle_on_pause_ss;
        critical_idle_ss = 0;
        critical_idle_count = 0;
        when_started_dt = _when_started_dt;
        last_iteration_dt = _last_iteration_dt;
        temperature_condition_offset_C = _temperature_condition_offset_C;
        
        // Запускаем исполнение
        states = IS_RUNNING;
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
    unsigned long total_in_process_ss(const TaskInstruction instructions[]) const {
        unsigned long total = 0;
        for (unsigned char i = 0; i <= current_instruction_index && i < total_instructions_count; ++i)
            total += instructions[i].get_in_process_ss();
        return total;
    }
    
    // Возвращает время total_in_process_ss в формате "hhh:mm:ss"
    const char* total_in_process_ss_to_string(const TaskInstruction instructions[]) const {
        static char buffer[16];
        unsigned long total_sec = total_in_process_ss(instructions);
        unsigned long hours = total_sec / 3600;
        unsigned long minutes = (total_sec % 3600) / 60;
        unsigned long seconds = total_sec % 60;
        if (hours < 100)
            snprintf(buffer, sizeof(buffer), "%02lu:%02lu:%02lu", hours, minutes, seconds);
        else
            snprintf(buffer, sizeof(buffer), "%03lu:%02lu:%02lu", hours, minutes, seconds);
        return buffer;
    }
    
    // Методы для увеличения in_process_ss и уменьшения duration_aim_left_ss для текущей инструкции
    void instruction_in_process_ss_inc(TaskInstruction instructions[]) {
        if (current_instruction_index < total_instructions_count)
            instructions[current_instruction_index].increase_in_process_ss();
    }
    void instruction_duration_aim_left_ss_dec(TaskInstruction instructions[]) {
        if (current_instruction_index < total_instructions_count) {
            if (instructions[current_instruction_index].duration_aim_left_ss > 0)
                --instructions[current_instruction_index].duration_aim_left_ss;
        }
    }
    
    // Сохранение состояния во FRAM (вызывается раз в 1000 мс)
    void save_story_callback() {
        // Реализуйте сохранение данных во FRAM по необходимости
    }
    
    // Методы переключения паузы
    void pause_by_user_plc_button() {
        set_is_on_pause_by_user(!is_on_pause_by_user());
        if (!is_on_pause_by_user() &&
            !is_on_pause_by_water_jacket_drain() &&
            !is_on_pause_by_no_380v())
        {
            set_is_on_pause(false);
        } else {
            set_is_on_pause(true);
        }
    }
    void pause_by_water_jacket_drain(bool flag) {
        set_is_on_pause_by_water_jacket_drain(flag);
        if(flag)
            set_is_on_pause(true);
        else if (!is_on_pause_by_user() && !is_on_pause_by_no_380v())
            set_is_on_pause(false);
    }
    void pause_by_no_380v(bool flag) {
        set_is_on_pause_by_no_380v(flag);
        if(flag)
            set_is_on_pause(true);
        else if (!is_on_pause_by_user() && !is_on_pause_by_water_jacket_drain())
            set_is_on_pause(false);
    }
    
    // Подтверждение завершения (без ошибки)
    void confirm_as_completed() {
        set_is_running(false);
        set_is_completed(true);
        save_story_callback();
        // Выключение физических устройств:
        // PhysicalController::instance()->turn_on_heaters(false);
        // PhysicalController::instance()->turn_on_water_jacket_valve(false);
        // PhysicalController::instance()->set_motor_rotation_speed_per_min(0, false);
    }
    // Завершение с ошибкой
    void confirm_as_completed_with_error(TaskExecutorError error) {
        set_is_running(false);
        set_is_completed_with_error(true);
        save_story_callback();
        // Выключение физических устройств:
        // PhysicalController::instance()->turn_on_heaters(false);
        // PhysicalController::instance()->turn_on_water_jacket_valve(false);
        // PhysicalController::instance()->set_motor_rotation_speed_per_min(0, false);
    }
    
    // Метод перехода к следующей инструкции (при подтверждении пользователем)
    void accept_instruction_as_completed_by_user(TaskInstruction instructions[]) {
        if (current_instruction_index < total_instructions_count - 1)
            ++current_instruction_index;
        else
            confirm_as_completed();
    }
    
    // Изменение максимального времени простоя в паузе
    void any_pause_max_idle_duration_ss(unsigned long idle_limit_ss) {
        max_idle_on_pause_ss = idle_limit_ss;
    }
    
    // Установка допускового значения температуры (например, ±5°C)
    void set_temperature_condition_offset(short offset) {
        temperature_condition_offset_C = offset;
    }
    
    // Полностью реализованный update(), вызываемый каждые 250 мс (или немедленно, если execute_immediately==true)
    // Обратите внимание: массив инструкций передаётся извне
    void update(TaskInstruction instructions[],
                DateTime rt_dt,
                bool have_water_in_water_jacket,
                bool have_380V_power,
                bool mixer_motor_crush,
                bool is_fast_mixer_motor,
                short temperature_C_product,
                short temperature_C_water_jacket = -255,
                bool execute_immediately = false)
    {
        // Если исполнитель не запущен – раз в 10 сек отключаем физические устройства
        static unsigned long last_disable_ms = 0;
        unsigned long currentMillis = millis();
        if (!is_running()) {
            if (currentMillis - last_disable_ms >= 10000 || execute_immediately) {
                // Пример вызова физических команд:
                // PhysicalController::instance()->turn_on_heaters(false);
                // PhysicalController::instance()->turn_on_water_jacket_valve(false);
                // PhysicalController::instance()->set_motor_rotation_speed_per_min(0, is_fast_mixer_motor);
                last_disable_ms = currentMillis;
            }
            return;
        }
        
        // Используем static переменные для таймеров внутри update()
        static unsigned long last_250ms_ms = 0;
        static unsigned long last_1000ms_ms = 0;
        static unsigned long water_jacket_timer_start = 0;
        
        // Если не execute_immediately – обновляем логику раз в 250 мс
        if (!execute_immediately && (currentMillis - last_250ms_ms < 250))
            return;
        last_250ms_ms = currentMillis;
        
        // Вычисляем разницу между rt_dt и последней итерацией (в секундах)
        TimeSpan diff = rt_dt - last_iteration_dt;
        int diff_sec = diff.total_seconds_abs();
        if(diff_sec < 5) {
            last_iteration_dt = rt_dt;
            return;
        } else if(diff_sec >= 5 && diff_sec <= 1800) {
            critical_idle_ss += diff_sec;
            ++critical_idle_count;
        } else if(diff_sec >= 1800) {
            confirm_as_completed_with_error(ErrorLongIdleByUserPause);
            return;
        }
        
        // Если включены какие-либо паузы – проверяем время простоя
        if(is_on_pause()) {
            if(critical_idle_ss >= max_idle_on_pause_ss) {
                if(is_on_pause_by_user())
                    confirm_as_completed_with_error(ErrorLongIdleByUserPause);
                else if(is_on_pause_by_no_380v())
                    confirm_as_completed_with_error(ErrorLongIdleByNo380v);
                else if(is_on_pause_by_water_jacket_drain())
                    confirm_as_completed_with_error(ErrorLongIdleByNoWaterInJacket);
                return;
            }
            // Если в паузе – останавливаем таймеры и физические устройства
            // (физ. команды ниже – примеры)
            // PhysicalController::instance()->turn_on_heaters(false);
            // PhysicalController::instance()->turn_on_water_jacket_valve(false);
            // PhysicalController::instance()->set_motor_rotation_speed_per_min(0, is_fast_mixer_motor);
            last_iteration_dt = rt_dt;
            return;
        }
        
        // Обработка воды в рубашке: если воды нет, включаем клапан и запускаем 30-секундный таймер
        if(!have_water_in_water_jacket) {
            // Включаем клапан для набора воды
            // PhysicalController::instance()->turn_on_water_jacket_valve(true);
            if(water_jacket_timer_start == 0)
                water_jacket_timer_start = currentMillis;
            if(currentMillis - water_jacket_timer_start >= 30000) {
                // Если воды так и не появилось – переводим в паузу по сливу рубашки
                pause_by_water_jacket_drain(true);
            }
        } else {
            // Если вода появилась – отключаем клапан и сбрасываем таймер
            // PhysicalController::instance()->turn_on_water_jacket_valve(false);
            water_jacket_timer_start = 0;
            if(is_on_pause_by_water_jacket_drain())
                pause_by_water_jacket_drain(false);
        }
        
        // Проверка 380V
        if(!have_380V_power)
            pause_by_no_380v(true);
        else if(is_on_pause_by_no_380v())
            pause_by_no_380v(false);
        
        // Проверка аварии мотора
        if(mixer_motor_crush) {
            confirm_as_completed_with_error(ErrorMixerCrash);
            return;
        }
        
        // Обработка текущей инструкции
        TaskInstruction& currentInst = instructions[current_instruction_index];
        
        // Если инструкция находится в очереди, переводим её в режим исполнения
        if (currentInst.get_is_in_queue()) {
            currentInst.set_is_in_queue(false);
            currentInst.set_is_in_process(true);
        }
        
        // Логика для этапа набора воды
        if (currentInst.get_is_water_intake_step()) {
            if (have_water_in_water_jacket) {
                currentInst.set_is_completed(true);
                currentInst.set_is_in_process(false);
                accept_instruction_as_completed_by_user(instructions);
                // После перехода на следующую инструкцию – выходим из update()
                last_iteration_dt = rt_dt;
                return;
            }
            // Если воды нет – клапан уже включён, ждём появления воды
        }
        
        // Обработка температуры и таймеров для инструкции
        // Если установлен ONLY_UNTIL_CONDITION_MET – переход по достижении температуры (с допуском)
        if (currentInst.get_is_only_until_condition_met()) {
            short target_temp = currentInst.temperature_C;
            if ( abs(temperature_C_product - target_temp) <= temperature_condition_offset_C ) {
                // Если температура достигнута – завершаем инструкцию
                currentInst.set_is_completed(true);
                currentInst.set_is_in_process(false);
                accept_instruction_as_completed_by_user(instructions);
                last_iteration_dt = rt_dt;
                return;
            } else {
                // Управление: если температура ниже – нагреваем, если выше – охлаждаем (при активном охлаждении)
                if (temperature_C_product < target_temp) {
                    // Включаем нагрев
                    // PhysicalController::instance()->turn_on_heaters(true);
                    if (currentInst.get_is_dont_rotate_on_pause() &&
                        currentInst.get_is_await_user_accept_when_completed())
                    {
                        // Если не вращать – устанавливаем 0 оборотов
                        // PhysicalController::instance()->set_motor_rotation_speed_per_min(0, is_fast_mixer_motor);
                    } else {
                        // Иначе устанавливаем скорость, заданную в инструкции
                        // PhysicalController::instance()->set_motor_rotation_speed_per_min(currentInst.rot_per_min, is_fast_mixer_motor);
                    }
                } else if (temperature_C_product > target_temp) {
                    if (currentInst.get_is_active_cooling()) {
                        // При охлаждении – отключаем нагрев
                        // PhysicalController::instance()->turn_on_heaters(false);
                        if (currentInst.get_is_dont_rotate_on_pause() &&
                            currentInst.get_is_await_user_accept_when_completed())
                        {
                            // Остановить мотор
                            // PhysicalController::instance()->set_motor_rotation_speed_per_min(0, is_fast_mixer_motor);
                        } else {
                            // Иначе скорость согласно инструкции
                            // PhysicalController::instance()->set_motor_rotation_speed_per_min(currentInst.rot_per_min, is_fast_mixer_motor);
                        }
                    }
                }
                // Обновляем таймеры и сохраняем состояние раз в 1000 мс
                if (currentMillis - last_1000ms_ms >= 1000) {
                    last_1000ms_ms = currentMillis;
                    instruction_duration_aim_left_ss_dec(instructions);
                    instruction_in_process_ss_inc(instructions);
                    save_story_callback();
                }
                // Если таймер истёк и флаг AWAIT_USER_ACCEPT не установлен – завершаем инструкцию
                if (currentInst.duration_aim_left_ss <= 0 &&
                    !currentInst.get_is_await_user_accept_when_completed())
                {
                    currentInst.set_is_completed(true);
                    currentInst.set_is_in_process(false);
                    accept_instruction_as_completed_by_user(instructions);
                    last_iteration_dt = rt_dt;
                    return;
                }
            }
        }
        else { // Если ONLY_UNTIL_CONDITION_MET не установлен – полагаемся на таймер
            if (currentMillis - last_1000ms_ms >= 1000) {
                last_1000ms_ms = currentMillis;
                instruction_duration_aim_left_ss_dec(instructions);
                instruction_in_process_ss_inc(instructions);
                save_story_callback();
            }
            if (currentInst.duration_aim_left_ss <= 0) {
                if (currentInst.get_is_await_user_accept_when_completed()) {
                    if (currentInst.get_is_dont_rotate_on_pause()) {
                        // Останавливаем мотор
                        // PhysicalController::instance()->set_motor_rotation_speed_per_min(0, is_fast_mixer_motor);
                    }
                    // Ждём подтверждения от пользователя (метод accept_instruction_as_completed_by_user() вызывается извне)
                } else {
                    currentInst.set_is_completed(true);
                    currentInst.set_is_in_process(false);
                    accept_instruction_as_completed_by_user(instructions);
                    last_iteration_dt = rt_dt;
                    return;
                }
            }
            else {
                // Пока таймер не истёк – управление нагревом/охлаждением аналогично:
                short target_temp = currentInst.temperature_C;
                if (temperature_C_product < target_temp) {
                    // Нагрев
                    // PhysicalController::instance()->turn_on_heaters(true);
                    if (currentInst.get_is_dont_rotate_on_pause() &&
                        currentInst.get_is_await_user_accept_when_completed())
                    {
                        // Остановка мотора
                        // PhysicalController::instance()->set_motor_rotation_speed_per_min(0, is_fast_mixer_motor);
                    } else {
                        // Установка скорости
                        // PhysicalController::instance()->set_motor_rotation_speed_per_min(currentInst.rot_per_min, is_fast_mixer_motor);
                    }
                } else if (temperature_C_product > target_temp) {
                    if (currentInst.get_is_active_cooling()) {
                        // Отключаем нагрев
                        // PhysicalController::instance()->turn_on_heaters(false);
                        if (currentInst.get_is_dont_rotate_on_pause() &&
                            currentInst.get_is_await_user_accept_when_completed())
                        {
                            // Остановка мотора
                            // PhysicalController::instance()->set_motor_rotation_speed_per_min(0, is_fast_mixer_motor);
                        } else {
                            // Установка скорости
                            // PhysicalController::instance()->set_motor_rotation_speed_per_min(currentInst.rot_per_min, is_fast_mixer_motor);
                        }
                    }
                }
            }
        }
        
        // Обновляем время последней итерации
        last_iteration_dt = rt_dt;
    }
};
#pragma pack(pop)

#endif // !T_TASK_EXECUTOR_H