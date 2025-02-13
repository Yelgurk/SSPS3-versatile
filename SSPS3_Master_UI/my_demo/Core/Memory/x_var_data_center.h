#pragma once
#ifndef X_VAR_DATA_CENTER_H
#define X_VAR_DATA_CENTER_H

#ifdef DEV_SSPS3_RUN_ON_PLC
    #include "./External/ext_mem_task_cluster.h"
    #include "./x_var_allocator.h"
#else
    #include "x_var_allocator.h"
    #include "ext_mem_task_cluster.h"
#endif

class XVarDataCenter
{
private:
    bool _ext_mem_vars_allocated = false;

    template<typename T>
    XVar<T>& _init(T default_value, signed int ext_mem_var_addr = 0, bool is_system_val = false, bool is_admin_val = false)
    {
        return XVarAllocator::instance()->allocate(default_value, ext_mem_var_addr, is_system_val, is_admin_val);
    }

    XVarDataCenter()
    {
        TestStore = std::make_unique<ExtMemTaskCluster>();
        TestStore.get()->create_page(TV_1_page_name)
            ->push(TV_1_name, TV_1_rpm, TV_1_temp_c, TV_1_durat_ss, TV_1_is_active_cooling, TV_1_is_await_user_ok, TV_1_is_stage_turned_on)
            ->push(TV_2_name, TV_2_rpm, TV_2_temp_c, TV_2_durat_ss, TV_2_is_active_cooling, TV_2_is_await_user_ok, TV_2_is_stage_turned_on)
            ->push(TV_3_name, TV_3_rpm, TV_3_temp_c, TV_3_durat_ss, TV_3_is_active_cooling, TV_3_is_await_user_ok, TV_3_is_stage_turned_on);

        TestStore.get()->create_page(TV_2_page_name)
            ->push(TV_4_name, TV_4_rpm, TV_4_temp_c, TV_4_durat_ss, TV_4_is_active_cooling, TV_4_is_await_user_ok, TV_4_is_stage_turned_on)
            ->push(TV_5_name, TV_5_rpm, TV_5_temp_c, TV_5_durat_ss, TV_5_is_active_cooling, TV_5_is_await_user_ok, TV_5_is_stage_turned_on);

        TestStore.get()->create_page(TV_3_page_name)
            ->push(TV_6_name, TV_6_rpm, TV_6_temp_c, TV_6_durat_ss, TV_6_is_active_cooling, TV_6_is_await_user_ok, TV_6_is_stage_turned_on);
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
        ..._s_... - system value, which ones is used for configuration any sensors, like example
        ..._a_... - admin value, whuch ones set plc/product behaviour
        ...       - without prefix. Just any template/task vars
*/
    //XVar<DataStageTemplate> TestVarTempl1;

    XVar<short>& TestVar1 = _init<short>(0);
    XVar<short>& TestVar2 = _init<short>(0);
    XVar<short>& TestVar3 = _init<short>(0);

    XVar<std::string>&    TV_1_page_name          = _init<std::string>("page 1");
    XVar<std::string>&    TV_2_page_name          = _init<std::string>("page 2");
    XVar<std::string>&    TV_3_page_name          = _init<std::string>("page 3");

    XVar<std::string>&    TV_1_name               = _init<std::string>("templ 1");
    XVar<unsigned char>&  TV_1_rpm                = _init<unsigned char>(2);
    XVar<unsigned char>&  TV_1_temp_c             = _init<unsigned char>(2);
    XVar<unsigned short>& TV_1_durat_ss           = _init<unsigned short>(2);
    XVar<bool>&           TV_1_is_active_cooling  = _init<bool>(0);
    XVar<bool>&           TV_1_is_await_user_ok   = _init<bool>(0);
    XVar<bool>&           TV_1_is_stage_turned_on = _init<bool>(0);

    XVar<std::string>&    TV_2_name               = _init<std::string>("templ 2");
    XVar<unsigned char>&  TV_2_rpm                = _init<unsigned char>(4);
    XVar<unsigned char>&  TV_2_temp_c             = _init<unsigned char>(4);
    XVar<unsigned short>& TV_2_durat_ss           = _init<unsigned short>(4);
    XVar<bool>&           TV_2_is_active_cooling  = _init<bool>(1);
    XVar<bool>&           TV_2_is_await_user_ok   = _init<bool>(1);
    XVar<bool>&           TV_2_is_stage_turned_on = _init<bool>(1);

    XVar<std::string>&    TV_3_name               = _init<std::string>("templ 3");
    XVar<unsigned char>&  TV_3_rpm                = _init<unsigned char>(6);
    XVar<unsigned char>&  TV_3_temp_c             = _init<unsigned char>(6);
    XVar<unsigned short>& TV_3_durat_ss           = _init<unsigned short>(6);
    XVar<bool>&           TV_3_is_active_cooling  = _init<bool>(0);
    XVar<bool>&           TV_3_is_await_user_ok   = _init<bool>(0);
    XVar<bool>&           TV_3_is_stage_turned_on = _init<bool>(0);

    XVar<std::string>&    TV_4_name               = _init<std::string>("templ 4");
    XVar<unsigned char>&  TV_4_rpm                = _init<unsigned char>(80);
    XVar<unsigned char>&  TV_4_temp_c             = _init<unsigned char>(80);
    XVar<unsigned short>& TV_4_durat_ss           = _init<unsigned short>(80);
    XVar<bool>&           TV_4_is_active_cooling  = _init<bool>(1);
    XVar<bool>&           TV_4_is_await_user_ok   = _init<bool>(1);
    XVar<bool>&           TV_4_is_stage_turned_on = _init<bool>(1);

    XVar<std::string>&    TV_5_name               = _init<std::string>("templ 5");
    XVar<unsigned char>&  TV_5_rpm                = _init<unsigned char>(100);
    XVar<unsigned char>&  TV_5_temp_c             = _init<unsigned char>(100);
    XVar<unsigned short>& TV_5_durat_ss           = _init<unsigned short>(100);
    XVar<bool>&           TV_5_is_active_cooling  = _init<bool>(0);
    XVar<bool>&           TV_5_is_await_user_ok   = _init<bool>(0);
    XVar<bool>&           TV_5_is_stage_turned_on = _init<bool>(0);

    XVar<std::string>&    TV_6_name               = _init<std::string>("templ 6");
    XVar<unsigned char>&  TV_6_rpm                = _init<unsigned char>(200);
    XVar<unsigned char>&  TV_6_temp_c             = _init<unsigned char>(200);
    XVar<unsigned short>& TV_6_durat_ss           = _init<unsigned short>(200);
    XVar<bool>&           TV_6_is_active_cooling  = _init<bool>(1);
    XVar<bool>&           TV_6_is_await_user_ok   = _init<bool>(1);
    XVar<bool>&           TV_6_is_stage_turned_on = _init<bool>(1);

    std::unique_ptr<ExtMemTaskCluster> TestStore;
};

#endif // !MEM_VAR_STORAGE_H
