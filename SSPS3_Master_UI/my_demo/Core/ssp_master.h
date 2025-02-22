#pragma once
#ifndef SSP_MASTER_H
#define SSP_MASTER_H

#ifdef DEV_SSPS3_RUN_ON_PLC

#include "./Memory/x_var.h"
#include "./Memory/Device/mem_i2c_fm24gl64.h"
#include "./Memory/Device/mem_spi_fm25cl64.h"
#include "./MCUsCommunication/MQTT/mqtt_i2c.h"
#include "./MCUsCommunication/MQTT/mqtt_spi.h"
#include "./Inpdev/matrix_keypad_enum.h"
#include "./ExternalEnvironment/io_physical_monitor.h"
#include "./BusinessLogic/liquid_dispenser.h"
#include "./BusinessLogic/task_instruction.h"
#include "./BusinessLogic/task_executor.h"
#include "./BusinessLogic/Provider/task_model_to_memory_provider.h"

#else

#include "x_var.h"
#include "mem_i2c_fm24gl64.h"
#include "mem_spi_fm25cl64.h"
#include "mqtt_i2c.h"
#include "mqtt_spi.h"
#include "matrix_keypad_enum.h"
#include "io_physical_monitor.h"
#include "liquid_dispenser.h"
#include "task_instruction.h"
#include "task_executor.h"
#include "task_model_to_memory_provider.h"

#endif

#endif // !SSP_MASTER_H