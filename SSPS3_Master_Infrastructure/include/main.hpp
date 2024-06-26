#include <Arduino.h>
#include "./STM32_slave.hpp"

#ifndef SSPS3_APPLICATION_SOLUTION
#ifndef SSPS3_UI_DEV_SOLUTION

#define SDA                         48
#define SCL                         47
#define INT                         12
#define STM_I2C_ADDR                0x30

extern TwoWire * itcw;
extern STM32_slave * STM32;

void print(bool only_digital = false);

uint8_t OptIn_state[8] = { 0 };

#endif
#endif

volatile bool interrupted_by_slave = false;
