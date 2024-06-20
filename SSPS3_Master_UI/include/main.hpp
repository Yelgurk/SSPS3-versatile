#include "UIService.hpp"
#include "./UIElements/global_ui_clock.hpp"
#include "../../SSPS3_Master_Infrastructure/include/main.hpp"

#ifndef SSPS3_APPLICATION_SOLUTION

#define SDA                         48
#define SCL                         47
#define INT                         12
#define STM_I2C_ADDR                0x30

extern TwoWire * itcw;
extern STM32_slave * STM32;

void print(bool only_digital = false);

uint8_t OptIn_state[8] = { 0 };

#endif

UIService UI_service;
UIClock * UI_clock;