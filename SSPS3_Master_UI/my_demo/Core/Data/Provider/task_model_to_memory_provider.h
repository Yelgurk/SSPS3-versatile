#pragma once
#ifndef TASK_MODEL_TO_MEMORY_PROVIDER_H
#define TASK_MODEL_TO_MEMORY_PROVIDER_H

#include <functional>
#include "../t_task_instruction.h"

typedef std::function<TaskInstruction*(unsigned char)> ExtractInstructionFromXVarArrayHandler;
typedef std::function<void(unsigned char)> SaveInstructionChangesInXVarArrayHandler;
typedef std::function<void()> SaveTaskExecutorChangesInXVar;

class TaskModelToMemoryProvider
{
private:
    TaskModelToMemoryProvider()
    {}

    ExtractInstructionFromXVarArrayHandler      _extractor_handler;
    SaveInstructionChangesInXVarArrayHandler    _save_instruction_handler;
    SaveTaskExecutorChangesInXVar               _save_task_executor_handler;
public:
    static TaskModelToMemoryProvider* instance()
    {
        static TaskModelToMemoryProvider inst;
        return &inst;
    }

    void set_task_instruction_extractor_handler(ExtractInstructionFromXVarArrayHandler extractor_handler) {
        this->_extractor_handler = extractor_handler;
    }

    void set_task_instruction_save_changes_handler(SaveInstructionChangesInXVarArrayHandler save_handler) {
        this->_save_instruction_handler = save_handler;
    }

    void set_task_executor_save_changes_handler(SaveTaskExecutorChangesInXVar save_handler) {
        this->_save_task_executor_handler = save_handler;
    }

    TaskInstruction* get_task_instruction(unsigned char index)
    {
        if (_extractor_handler && _save_instruction_handler)
            return _extractor_handler(index);

        return nullptr;
    }

    void save_task_instruction_changes(unsigned char index)
    {
        if (_extractor_handler && _save_instruction_handler)
            _save_instruction_handler(index);
    }

    void save_task_executor_changes()
    {
        if (_save_task_executor_handler)
            _save_task_executor_handler();
    }
};

#define TaskToMem   TaskModelToMemoryProvider::instance()

#endif