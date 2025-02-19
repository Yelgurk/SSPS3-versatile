#pragma once
#ifndef X_VAR_DATA_CENTER_H
#define X_VAR_DATA_CENTER_H

#ifdef DEV_SSPS3_RUN_ON_PLC
    #include "./External/ext_mem_task_cluster.h"
    #include "./x_var_array.h"
    #include "../Data/t_task_executor.h"
#else
    #include "ext_mem_task_cluster.h"
    #include "x_var_array.h"
    #include "t_task_executor.h"
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
        TestStore = std::make_unique<ExtMemTaskCluster>();

                //e_settings_task_names_array      
                //e_settings_task_RPMs_array             
                //e_settings_task_tempCs_array           
                //e_settings_task_ss_durations_array     
                //e_settings_task_is_active_cooling_array 
                //e_settings_task_is_user_ok_cooling_array
                //e_ettings_task_is_step_turned_on_array  

        uint8_t i = 0;

        TestStore.get()->create_page(TV_1_page_name);
        for (; i < 3; i++)
        {
            TestStore.get()->get_page()->push(
                e_settings_task_names_array[i],
                e_settings_task_RPMs_array[i],
                e_settings_task_tempCs_array[i],
                e_settings_task_ss_durations_array[i],
                e_settings_task_is_active_cooling_array[i],
                e_settings_task_is_user_ok_cooling_array[i],
                e_ettings_task_is_step_turned_on_array[i]
            );
        }

        TestStore.get()->create_page(TV_2_page_name);
        for (; i < 5; i++)
        {
            TestStore.get()->get_page()->push(
                e_settings_task_names_array[i],
                e_settings_task_RPMs_array[i],
                e_settings_task_tempCs_array[i],
                e_settings_task_ss_durations_array[i],
                e_settings_task_is_active_cooling_array[i],
                e_settings_task_is_user_ok_cooling_array[i],
                e_ettings_task_is_step_turned_on_array[i]
            );
        }

        TestStore.get()->create_page(TV_3_page_name);
        for (; i < 6; i++)
        {
            TestStore.get()->get_page()->push(
                e_settings_task_names_array[i],
                e_settings_task_RPMs_array[i],
                e_settings_task_tempCs_array[i],
                e_settings_task_ss_durations_array[i],
                e_settings_task_is_active_cooling_array[i],
                e_settings_task_is_user_ok_cooling_array[i],
                e_ettings_task_is_step_turned_on_array[i]
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

    2 lvl prefix:
        ..._s_... - system value, which ones is used for configuration any sensors
        ..._a_... - admin value, whuch ones set plc/product behaviour             
        ...       - without prefix. Just any template/task vars                   
*/
    //XVar<DataStageTemplate> TestVarTempl1;

    XVar<std::string> Startup_checksum = _init<std::string>("Krugley Maxim 12.09");

    XVarArray<short, 3>    TV_short_arr{ 3, 3, 3 };

    XVar<short>& TestVar1 = _init<short>(0);
    XVar<short>& TestVar2 = _init<short>(0);
    XVar<short>& TestVar3 = _init<short>(0);

    XVar<std::string>&    TV_1_page_name          = _init<std::string>("page 1");
    XVar<std::string>&    TV_2_page_name          = _init<std::string>("page 2");
    XVar<std::string>&    TV_3_page_name          = _init<std::string>("page 3");

    XVarArray<std::string, 6> e_settings_task_names_array       { "templ 1", "templ 2", "templ 3", "templ 4", "templ 5", "templ 6" };
    XVarArray<short, 6> e_settings_task_RPMs_array              { 2, 4, 6, 80, 100, 200 };
    XVarArray<short, 6> e_settings_task_tempCs_array            { 20, 30, 40, 50, 60, 70 };
    XVarArray<short, 6> e_settings_task_ss_durations_array      { 100, 120, 140, 160, 180, 200 };
    XVarArray<bool, 6> e_settings_task_is_active_cooling_array  { true, true, false, false, true, true };
    XVarArray<bool, 6> e_settings_task_is_user_ok_cooling_array { true, true, false, false, true, true };
    XVarArray<bool, 6> e_ettings_task_is_step_turned_on_array   { true, true, true, true, true, true} ;

    TaskInstruction l_dummy_task_instruction;
    XVarArray<TaskInstruction, 30>  e_task_instructions_array = XVarArray<TaskInstruction, 30>(l_dummy_task_instruction);

    TaskExecutor l_dummy_task_executor;
    XVar<TaskExecutor> e_task_executor            = _init<TaskExecutor>(l_dummy_task_executor);

    std::unique_ptr<ExtMemTaskCluster> TestStore;
};

#endif // !MEM_VAR_STORAGE_H
