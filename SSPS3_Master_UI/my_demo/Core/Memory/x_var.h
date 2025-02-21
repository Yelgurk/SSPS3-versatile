﻿#pragma once
#ifndef X_VAR_H
#define X_VAR_H

#ifdef DEV_SSPS3_RUN_ON_PLC
    #include "./x_var_data_center.h"
    #include "./Device/mem_i2c_fm24gl64.h"
#else
    #include "x_var_data_center.h"
    #include "mem_i2c_fm24gl64.h"
#endif

#define XStorage                        XVarDataCenter::get()
#define XStorageTaskPage(value)         XVarDataCenter::get()->c_task_cluster.get()->select_page(value)
#define XStorageTaskInstruction(value)  XVarDataCenter::get()->c_task_cluster.get()->select_instruction(value)
#define XStorageTaskInstructionActive   XVarDataCenter::get()->c_task_cluster.get()->get_instruction()
#define XStorageDispatcher              MemDevicesDispatcher::instance()
#define XAllocator                      XVarAllocator::instance()

#endif // !X_VAR_H
