#pragma once
#ifndef CONTAINER_TASK_INSTRUCTION_H
#define CONTAINER_TASK_INSTRUCTION_H

#ifdef DEV_SSPS3_RUN_ON_PLC
    #include <string>
    #include "../x_var_T_string.h"
    #include "../x_var_T_struct.h"
    #include "../x_var_T_arithmetic.h"
#else
    #include <string>
    #include "x_var_T_string.h"
    #include "x_var_T_struct.h"
    #include "x_var_T_arithmetic.h"
#endif

class ContainerTaskInstruction
{
public:
    ContainerTaskInstruction(
        XVar<std::string>& name,
        XVar<short>& rpm,
        XVar<short>& temperature,
        XVar<short>& duration_ss,
        XVar<bool>& is_active_cooling,
        XVar<bool>& is_await_user_ok,
        XVar<bool>& is_stage_turned_on) :
        name(name),
        rpm(rpm),
        temperature(temperature),
        duration_ss(duration_ss),
        is_active_cooling(is_active_cooling),
        is_await_user_ok(is_await_user_ok),
        is_stage_turned_on(is_stage_turned_on)
    {}

    XVar<std::string>& name;
    XVar<short>& rpm;
    XVar<short>& temperature;
    XVar<short>& duration_ss;
    XVar<bool>& is_active_cooling;
    XVar<bool>& is_await_user_ok;
    XVar<bool>& is_stage_turned_on;

    void load_into_ui()
    {
        name                .load_value_into_ui();
        rpm                 .load_value_into_ui();
        temperature         .load_value_into_ui();
        duration_ss         .load_value_into_ui();
        is_active_cooling   .load_value_into_ui();
        is_await_user_ok    .load_value_into_ui();
        is_stage_turned_on  .load_value_into_ui();
    }
};

#endif // !CONTAINER_TASK_INSTRUCTION_H
