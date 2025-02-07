#pragma once
#ifndef X_VAR_T_H
#define X_VAR_T_H

#ifdef DEV_SSPS3_RUN_ON_PLC
    #include "./x_var_base.h"
#else
    #include "x_var_base.h"
#endif

template <typename T, typename Enable = void>
class XVar;

#endif // !X_VAR_T_H
