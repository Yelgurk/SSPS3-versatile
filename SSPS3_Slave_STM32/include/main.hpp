#include <Arduino.h>
#include "Keypad.h"
#include "./I2C_Service.hpp"

#define SDA             PB7
#define SCL             PB6
#define INT             PB8
#define STM_I2C_ADR     0x30

#define ARR_SIZE(arr, type)         (sizeof(arr) / sizeof(type))
#define PIN_EXISTS(pin, arr, type)  InRange(pin, ARR_SIZE(arr, type))
#define INTERRUPT_MASTER(val)       digitalWrite(INT, val)       

#define KB_Row                      4
#define KB_Col                      4
#define KB_Await                    (KB_Row * KB_Col)

uint8_t keys[KB_Row][KB_Col] =
{
    {0, 1, 2, 3},
    {4, 5, 6, 7},
    {8, 9, 10, 11},
    {12, 13, 14, 15}
};

uint8_t KB_row_pin[KB_Row] = {PB11, PB13, PA9, PA11};
uint8_t KB_col_pin[KB_Col] = {PC7, PC9, PB14, PC6};

Keypad kpd = Keypad(makeKeymap(keys), KB_row_pin, KB_col_pin, KB_Row, KB_Col);

#define STM_RELAY_1     PB1
#define STM_RELAY_2     PB10
#define STM_RELAY_3     PB12
#define STM_RELAY_4     PB15
#define STM_RELAY_5     PA12
#define STM_RELAY_6     PA10
#define STM_RELAY_7     PA8
#define STM_RELAY_8     PC8

#define STM_DAC_1       PA4

#define STM_OPT_IN_1    PB0
#define STM_OPT_IN_2    PC5
#define STM_OPT_IN_3    PC4
#define STM_OPT_IN_4    PA7
#define STM_OPT_IN_5    PA6
#define STM_OPT_IN_6    PA5
#define STM_OPT_IN_7    PA3
#define STM_OPT_IN_8    PA2

#define STM_ADC_1       PC0
#define STM_ADC_2       PC1
#define STM_ADC_3       PC2
#define STM_ADC_4       PC3

uint8_t Relay[8]
{
    STM_RELAY_1,
    STM_RELAY_2,
    STM_RELAY_3,
    STM_RELAY_4,
    STM_RELAY_5,
    STM_RELAY_6,
    STM_RELAY_7,
    STM_RELAY_8
};

uint8_t Dac[1]
{
    STM_DAC_1
};

uint8_t OptIn[8]
{
    STM_OPT_IN_1,
    STM_OPT_IN_2,
    STM_OPT_IN_3,
    STM_OPT_IN_4,
    STM_OPT_IN_5,
    STM_OPT_IN_6,
    STM_OPT_IN_7,
    STM_OPT_IN_8
};

uint8_t Adc[4]
{
    STM_ADC_1,
    STM_ADC_2,
    STM_ADC_3,
    STM_ADC_4
};

uint8_t OptIn_state[ARR_SIZE(OptIn, uint8_t)] = { 0 };
uint8_t KeyPressed = KB_Await;

TwoWire * itcw;
I2C_Service * I2C;

uint8_t InRange(uint8_t value, uint8_t max) {
    return value >= max ? max - 1 : (value < 0 ? 0 : value);
}