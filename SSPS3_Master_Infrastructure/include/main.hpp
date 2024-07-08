#include "./STM32_slave.hpp"

volatile bool interrupted_by_slave = false;

	
#ifndef SSPS3_APPLICATION_SOLUTION
#define UNLOCK_INFRASTRUCTURE_MAIN_CPP
#include <Arduino.h>

#define SDA                         48
#define SCL                         47
#define INT                         12
#define STM_I2C_ADDR                0x30

extern TwoWire * itcw;
extern STM32_slave * STM32;

void print(bool only_digital = false);

uint8_t OptIn_state[8] = { 0 };

#endif