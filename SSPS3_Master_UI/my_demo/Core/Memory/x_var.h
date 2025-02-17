#pragma once
#ifndef X_VAR_H
#define X_VAR_H

#ifdef DEV_SSPS3_RUN_ON_PLC
    #include "./x_var_data_center.h"
    #include "./Device/mem_i2c_fm24gl64.h"
#else
    #include "x_var_data_center.h"
    #include "mem_i2c_fm24gl64.h"
#endif

#define XStorage                    XVarDataCenter::get()
#define XStorageTaskPage(value)     XVarDataCenter::get()->TestStore.get()->select_page(value)
#define XStorageTaskGroup(value)    XVarDataCenter::get()->TestStore.get()->select_group(value)
#define XStorageTaskGet             XVarDataCenter::get()->TestStore.get()->get_group()
#define XStorageDispatcher          ExtMemDevicesDispatcher::instance()
#define XAllocator                  XVarAllocator::instance()

#endif // !X_VAR_H
