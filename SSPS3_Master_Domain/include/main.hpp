#include <Arduino.h>
#include "./STM32_slave.hpp"
#include "./FRAM_Var.hpp"
#include "./FRAM_allocator.hpp"

#define SDA                         48
#define SCL                         47
#define INT                         12
#define STM_I2C_ADDR                0x30
#define FRAM_I2C_ADDR               0x50
#define FRAM_ALLOC(type)            FRAM_allocator::getAddr(sizeof(type))
#define FRAM_ALLOC_ADDR(type, addr) FRAM_allocator::getAddr(sizeof(type), addr)

extern uint8_t FRAM_address;
extern TwoWire * itcw;
extern STM32_slave * STM32;

uint8_t OptIn_state[8] = { 0 };

FRAM_Var<boolean>       mem_Timer1(123, FRAM_ALLOC(boolean));
FRAM_Var<uint8_t>       mem_Timer2(123, FRAM_ALLOC(uint8_t));
FRAM_Var<uint16_t>      mem_Timer3(123, FRAM_ALLOC(uint16_t));
FRAM_Var<uint32_t>      mem_Timer4(123, FRAM_ALLOC_ADDR(uint32_t, 100));
FRAM_Var<uint32_t>      mem_Timer5(123, FRAM_ALLOC(uint32_t));