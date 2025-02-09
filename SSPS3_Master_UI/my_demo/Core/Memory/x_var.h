#pragma once
#ifndef X_VAR_CORE_H
#define X_VAR_CORE_H

#ifdef DEV_SSPS3_RUN_ON_PLC
    #include "./x_var_data_center.h"
    #include "./Device/mem_i2c_fm24gl64.h"
#else
    #include "x_var_data_center.h"
    #include "mem_i2c_fm24gl64.h"
#endif

#endif // !X_VAR_CORE_H