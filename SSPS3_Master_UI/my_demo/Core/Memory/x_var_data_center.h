#pragma once
#ifndef X_VAR_DATA_CENTER_H
#define X_VAR_DATA_CENTER_H

#ifdef DEV_SSPS3_RUN_ON_PLC
    #include "./x_var_array.h"
    #include "./Container/container_task_cluster.h"
    #include "../BusinessLogic/task_executor.h"
#else
    #include "x_var_array.h"
    #include "container_task_cluster.h"
    #include "task_executor.h"
#endif

class XVarDataCenter
{
private:
    bool _ext_mem_vars_allocated = false;

    template<typename T>
    XVar<T>& _init(T default_value, signed int ext_mem_var_addr = 0, bool is_system_val = false, bool is_admin_val = false)
    {
        return XVarAllocator::instance()->allocate<T>(default_value, ext_mem_var_addr, is_system_val, is_admin_val);
    }

    XVarDataCenter()
    {
        c_task_cluster = std::make_unique<ContainerTaskCluster>();

        uint8_t i = 0;

        c_task_cluster.get()->create_page(e_array_settings_task_names[0]);
        for (; i < 3; i++)
        {
            c_task_cluster.get()->get_page()->push(
                e_array_settings_instruction_names[i],
                e_array_settings_task_RPMs[i],
                e_array_settings_task_tempCs[i],
                e_array_settings_task_ss_durations[i],
                e_array_settings_task_is_active_cooling[i],
                e_array_settings_task_is_user_ok_cooling[i],
                e_array_settings_task_is_step_turned_on[i]
            );
        }

        c_task_cluster.get()->create_page(e_array_settings_task_names[1]);
        for (; i < 5; i++)
        {
            c_task_cluster.get()->get_page()->push(
                e_array_settings_instruction_names[i],
                e_array_settings_task_RPMs[i],
                e_array_settings_task_tempCs[i],
                e_array_settings_task_ss_durations[i],
                e_array_settings_task_is_active_cooling[i],
                e_array_settings_task_is_user_ok_cooling[i],
                e_array_settings_task_is_step_turned_on[i]
            );
        }

        c_task_cluster.get()->create_page(e_array_settings_task_names[2]);
        for (; i < 6; i++)
        {
            c_task_cluster.get()->get_page()->push(
                e_array_settings_instruction_names[i],
                e_array_settings_task_RPMs[i],
                e_array_settings_task_tempCs[i],
                e_array_settings_task_ss_durations[i],
                e_array_settings_task_is_active_cooling[i],
                e_array_settings_task_is_user_ok_cooling[i],
                e_array_settings_task_is_step_turned_on[i]
            );
        }

        TaskToMem->set_task_instruction_extractor_handler([](unsigned char _index){
            return &XVarDataCenter::get()->e_task_instructions_array[_index].get();
        });

        TaskToMem->set_task_instruction_save_changes_handler([](unsigned char _index){
            XVarDataCenter::get()->e_task_instructions_array[_index].save_changes();
        });

        TaskToMem->set_task_executor_save_changes_handler([](){
            XVarDataCenter::get()->e_task_executor.save_changes();
        });
    }

public:
    static XVarDataCenter* get()
    {
        static XVarDataCenter inst;
        return &inst;
    }

    void test_reset_all_vals()
    {
        XVarAllocator::instance()->vars_full_hard_reset();
    }

/*
    1 lvl prefix:
        e_... - placed in external memory (like i2c device fm24cl64)
        l_... - not allocated in ext memory. Only in RAM
        c_... - local container for external far-s

    2 lvl prefix:
        ..._s_... - system value, which ones is used for configuration any sensors
        ..._a_... - admin value, whuch ones set plc/product behaviour             
        ...       - without prefix. Just any template/task vars                   
*/
    XVar<std::string> Startup_checksum = _init<std::string>("Krugley Maxim 12.09");

    std::unique_ptr<ContainerTaskCluster> c_task_cluster;
    XVarArray<std::string, 3> e_array_settings_task_names       { "task 1", "task 2", "task 3" };
    XVarArray<std::string, 6> e_array_settings_instruction_names{ "templ 1", "templ 2", "templ 3", "templ 4", "templ 5", "templ 6" };
    XVarArray<short, 6> e_array_settings_task_RPMs              { 2, 4, 6, 80, 100, 200 };
    XVarArray<short, 6> e_array_settings_task_tempCs            { 20, 30, 40, 50, 60, 70 };
    XVarArray<short, 6> e_array_settings_task_ss_durations      { 100, 120, 140, 160, 180, 200 };
    XVarArray<bool, 6> e_array_settings_task_is_active_cooling  { true, true, false, false, true, true };
    XVarArray<bool, 6> e_array_settings_task_is_user_ok_cooling { true, true, false, false, true, true };
    XVarArray<bool, 6> e_array_settings_task_is_step_turned_on  { true, true, true, true, true, true} ;

    TaskInstruction l_dummy_task_instruction;
    XVarArray<TaskInstruction, 30>  e_task_instructions_array = XVarArray<TaskInstruction, 30>(l_dummy_task_instruction);

    TaskExecutor l_dummy_task_executor;
    XVar<TaskExecutor> e_task_executor = _init<TaskExecutor>(l_dummy_task_executor);    
};

#endif // !MEM_VAR_STORAGE_H
