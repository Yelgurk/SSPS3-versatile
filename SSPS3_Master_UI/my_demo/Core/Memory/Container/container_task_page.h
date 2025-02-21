#pragma once
#ifndef EXT_MEM_TASK_PAGE_H
#define EXT_MEM_TASK_PAGE_H

#ifdef DEV_SSPS3_RUN_ON_PLC
    #include <string>
    #include <list>
    #include "./container_task_instruction.h"
#else
    #include <string>
    #include <vector>
    #include "container_task_instruction.h"
#endif

class ContainerTaskPage
{
private:
    std::list<ContainerTaskInstruction>::iterator l_front;
    std::list<ContainerTaskInstruction> instructions;
    ContainerTaskInstruction* _selected_instruction;

    XVar<std::string>& page_name;

public:
    ContainerTaskPage(XVar<std::string>& page_name) :
        page_name(page_name),
        _selected_instruction(nullptr)
    {}

    ContainerTaskPage* push(
        XVar<std::string>& name,
        XVar<short>& rpm,
        XVar<short>& temperature,
        XVar<short>& duration_ss,
        XVar<bool>& is_active_cooling,
        XVar<bool>& is_await_user_ok,
        XVar<bool>& is_stage_turned_on)
    {
        instructions.push_back(ContainerTaskInstruction(
            name,
            rpm,
            temperature,
            duration_ss,
            is_active_cooling,
            is_await_user_ok,
            is_stage_turned_on
        ));
        _selected_instruction = &instructions.back();

        return this;
    }

    ContainerTaskPage* select_instruction(unsigned char index)
    {
        std::advance(
            l_front = instructions.begin(),
            index >= instructions.size() ? instructions.size() - 1 : index
        );

        _selected_instruction = &(*l_front);
        _selected_instruction->load_into_ui();

        page_name.load_value_into_ui();
        return this;
    }

    ContainerTaskInstruction* get_instruction()
    {
        return _selected_instruction;
    }
};

#endif // ! MEM_EXT_STAGE_TEMPLATE_H
