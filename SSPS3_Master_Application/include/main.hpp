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

FRAM_Var<boolean>       mem_Timer1(123, FRAM_ALLOC(boolean));
FRAM_Var<uint8_t>       mem_Timer2(123, FRAM_ALLOC(uint8_t));
FRAM_Var<uint16_t>      mem_Timer3(123, FRAM_ALLOC(uint16_t));
FRAM_Var<uint32_t>      mem_Timer4(123, FRAM_ALLOC_ADDR(uint32_t, 100));
FRAM_Var<uint32_t>      mem_Timer5(123, FRAM_ALLOC(uint32_t));

uint8_t OptIn_state[8] = { 0 };