#include <Arduino.h>
#include "../../SSPS3_Master_Domain/include/main.hpp"
#include "../../SSPS3_Master_Infrastructure/include/main.hpp"

#define SDA                         48
#define SCL                         47
#define INT                         12
#define STM_I2C_ADDR                0x30
#define FRAM_I2C_ADDR               0x50

extern uint8_t FRAM_address;
extern TwoWire * itcw;
extern STM32_slave * STM32;