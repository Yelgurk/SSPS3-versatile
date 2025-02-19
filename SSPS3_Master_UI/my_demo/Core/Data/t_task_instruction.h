#pragma once
#ifndef T_TASK_INSTRUCTION_H
#define T_TASK_INSTRUCTION_H

#include <string.h>

#pragma pack(push, 1)
struct TaskInstruction {
    char name[20];
    short rot_per_min;
    short temperature_C;          // целевая температура в градусах Цельсия
    short duration_aim_ss;        // целевое время работы (секунд)
    short duration_aim_left_ss;   // оставшееся время работы (секунд)
    unsigned char configurations; // битовое поле для bool-параметров
    unsigned char states;         // битовое поле для состояний
    unsigned long in_process_ss;  // время исполнения в секундах

    // Битовые маски для configurations
    enum ConfigBits {
        WATER_INTAKE_STEP         = 1 << 0, // этап набора воды (набор воды, если её нет)
        ONLY_UNTIL_CONDITION_MET  = 1 << 1, // выполнять только до достижения условия (температура)
        ACTIVE_COOLING            = 1 << 2, // включать активное охлаждение, если температура выше
        AWAIT_USER_ACCEPT         = 1 << 3, // требуется подтверждение пользователя для перехода
        LAST_STEP                 = 1 << 4, // последняя инструкция (логика отличается – только AWAIT_USER_ACCEPT)
        COOLING_AT_END            = 1 << 5, // (флаг для будущей логики, не участвует)
        HEATING_AT_END            = 1 << 6, // (флаг для будущей логики, не участвует)
        DONT_ROTATE_ON_PAUSE      = 1 << 7  // при паузе не вращать мотор, если и AWAIT_USER_ACCEPT установлено
    };

    // Битовые маски для states
    enum StateBits {
        COMPLETED   = 1 << 0, // инструкция выполнена
        IN_PROCESS  = 1 << 1, // инструкция выполняется
        IN_QUEUE    = 1 << 2, // инструкция находится в очереди
        CANCELLED   = 1 << 3  // инструкция отменена
    };

    // Конструктор по умолчанию – все поля обнуляются
    TaskInstruction() : rot_per_min(0), temperature_C(0), duration_aim_ss(0),
                    duration_aim_left_ss(0), configurations(0), states(0), in_process_ss(0)
    {
        name[0] = '\0';
    }
    
    // Метод инициализации, выполняющий всю логику, ранее размещённую в конструкторе
    void init(const char* _name,
              short _rot_per_min,
              short _temperature_C,
              short _duration_aim_ss,
              bool is_water_intake_step,
              bool is_only_until_condition_met,
              bool is_active_cooling = true,
              bool is_await_user_accept_when_completed = false,
              bool is_last_step = false,
              bool is_cooling_at_the_end = false,
              bool is_heating_at_the_end = false,
              bool is_dont_rotate_on_user_await = false)
    {
        strncpy(name, _name, sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';
        
        rot_per_min = _rot_per_min;
        temperature_C = _temperature_C;
        duration_aim_ss = is_only_until_condition_met ? 0 : _duration_aim_ss;
        duration_aim_left_ss = duration_aim_ss;
        in_process_ss = 0;
        configurations = 0;
        states = 0;
        
        set_is_water_intake_step(is_water_intake_step);
        set_is_only_until_condition_met(is_only_until_condition_met);
        set_is_active_cooling(is_active_cooling);
        set_is_await_user_accept_when_completed(is_await_user_accept_when_completed);
        set_is_last_step(is_last_step);
        set_is_cooling_at_the_end(is_cooling_at_the_end);
        set_is_heating_at_the_end(is_heating_at_the_end);
        set_is_dont_rotate_on_user_await(is_dont_rotate_on_user_await);
        
        set_is_in_queue(true);
    }
    
    // Методы get/set для конфигурационных флагов
    bool get_is_water_intake_step() const { return configurations & WATER_INTAKE_STEP; }
    void set_is_water_intake_step(bool value) {
        if (value)
            configurations |= WATER_INTAKE_STEP;
        else
            configurations &= ~WATER_INTAKE_STEP;
    }

    bool get_is_only_until_condition_met() const { return configurations & ONLY_UNTIL_CONDITION_MET; }
    void set_is_only_until_condition_met(bool value) {
        if (value)
            configurations |= ONLY_UNTIL_CONDITION_MET;
        else
            configurations &= ~ONLY_UNTIL_CONDITION_MET;
    }

    bool get_is_active_cooling() const { return configurations & ACTIVE_COOLING; }
    void set_is_active_cooling(bool value) {
        if (value)
            configurations |= ACTIVE_COOLING;
        else
            configurations &= ~ACTIVE_COOLING;
    }

    bool get_is_await_user_accept_when_completed() const { return configurations & AWAIT_USER_ACCEPT; }
    void set_is_await_user_accept_when_completed(bool value) {
        if (value)
            configurations |= AWAIT_USER_ACCEPT;
        else
            configurations &= ~AWAIT_USER_ACCEPT;
    }

    bool get_is_last_step() const { return configurations & LAST_STEP; }
    void set_is_last_step(bool value) {
        if (value)
            configurations |= LAST_STEP;
        else
            configurations &= ~LAST_STEP;
    }

    bool get_is_cooling_at_the_end() const { return configurations & COOLING_AT_END; }
    void set_is_cooling_at_the_end(bool value) {
        if (value)
            configurations |= COOLING_AT_END;
        else
            configurations &= ~COOLING_AT_END;
    }

    bool get_is_heating_at_the_end() const { return configurations & HEATING_AT_END; }
    void set_is_heating_at_the_end(bool value) {
        if (value)
            configurations |= HEATING_AT_END;
        else
            configurations &= ~HEATING_AT_END;
    }
    
    bool get_is_dont_rotate_on_user_await() const { return configurations & DONT_ROTATE_ON_PAUSE; }
    void set_is_dont_rotate_on_user_await(bool value) {
        if (value)
            configurations |= DONT_ROTATE_ON_PAUSE;
        else
            configurations &= ~DONT_ROTATE_ON_PAUSE;
    }
    
    // Методы get/set для состояний
    bool get_is_completed() const { return states & COMPLETED; }
    void set_is_completed(bool value) {
        if (value)
            states |= COMPLETED;
        else
            states &= ~COMPLETED;
    }

    bool get_is_in_process() const { return states & IN_PROCESS; }
    void set_is_in_process(bool value) {
        if (value)
            states |= IN_PROCESS;
        else
            states &= ~IN_PROCESS;
    }

    bool get_is_in_queue() const { return states & IN_QUEUE; }
    void set_is_in_queue(bool value) {
        if (value)
            states |= IN_QUEUE;
        else
            states &= ~IN_QUEUE;
    }

    bool get_is_cancelled() const { return states & CANCELLED; }
    void set_is_cancelled(bool value) {
        if (value)
            states |= CANCELLED;
        else
            states &= ~CANCELLED;
    }
    
    unsigned long get_in_process_ss() const { return in_process_ss; }
    void increase_in_process_ss() { ++in_process_ss; }

    unsigned long get_duration_aim_left_ss() const { return duration_aim_left_ss; }
    void decrease_duration_aim_left_ss()
    {
        if (!get_is_only_until_condition_met())
            --duration_aim_left_ss;
        else
            duration_aim_left_ss = 0;
    }

    bool operator==(const TaskInstruction& other) { return false; }
};
#pragma pack(pop)

#endif