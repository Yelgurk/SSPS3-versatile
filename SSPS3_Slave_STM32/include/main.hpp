#define KeyPadVersion   2

#include <Arduino.h>
#include <algorithm>
#include "./I2C_Service.hpp"


//#define SSPS3F1_V1
#define SSPS3F1_BLACKOUT_EDITION
//#define ALLOW_UART_DEBUG

// UART serial debug
#ifdef ALLOW_UART_DEBUG
    #define S_UART_TX   PA2
    #define S_UART_RX   PA3
#endif

/* Keypad 4x4 var's */
#define KB_Row                      4
#define KB_Col                      4
#define KB_Size                     (KB_Row * KB_Col)           
#define KB_Await                    (KB_Row * KB_Col) * 2
#define HOLD_begin_ms               1000
#define HOLD_x                      1.2
#define HOLD_min_ms                 50

uint8_t KB_row_pin[KB_Row] =
{
#ifdef SSPS3F1_V1
    PB11, PB13, PA9, PA11
#endif
#ifdef SSPS3F1_BLACKOUT_EDITION
    PA12, PA11, PC8, PC7
#endif
};

uint8_t KB_col_pin[KB_Col] =
{
#ifdef SSPS3F1_V1
    PC7, PC9, PB14, PC6
#endif
#ifdef SSPS3F1_BLACKOUT_EDITION
    PA8, PC9, PA10, PA9
#endif
};

uint8_t KeyNew = KB_Await;
uint8_t KeyPressed = KB_Await;
uint16_t KeyHoldDelay = HOLD_begin_ms;
uint64_t KeyHoldNext = 0;

#if KeyPadVersion == 1
    #include <Keypad.h>

    char keysInline[KB_Row * KB_Col] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P'};

    char keys[KB_Row][KB_Col] = {
        {'A', 'B', 'C', 'D'},
        {'E', 'F', 'G', 'H'},
        {'I', 'J', 'K', 'L'},
        {'M', 'N', 'O', 'P'}
    }; 

    Keypad kpd = Keypad(makeKeymap(keys), KB_row_pin, KB_col_pin, KB_Row, KB_Col);
#elif KeyPadVersion == 2
    #include "KeyPad4495.hpp"

    char keysInline[KB_Row * KB_Col] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

    Keypad4495 myKeypad(keysInline, KB_row_pin, KB_col_pin, KB_Row, KB_Col);
#endif

TwoWire * itcw;
I2C_Service * I2C;

/* I2C var's */
#define STM_I2C_ADR             0x30
#ifdef SSPS3F1_V1
    #define SDA                     PB7
    #define SCL                     PB6
    #define INT                     PB8
#endif
#ifdef SSPS3F1_BLACKOUT_EDITION
    #define SDA                     PB11
    #define SCL                     PB10
    #define INT                     PA6
    #define INT_KB                  PA5
#endif

/* STM32 slave calc func for i2c data transmittion */
#define ARR_SIZE(arr, type)         (sizeof(arr) / sizeof(type))
#define PIN_EXISTS(pin, arr, type)  InRange(pin, ARR_SIZE(arr, type))
//#define INTERRUPT_MASTER(val)       digitalWrite(INT, val)       
//#define IS_MASTER_INTERRUPTED       digitalRead(INT)
#define CHANGE_INT_SIGNAL()         digitalWrite(INT, !static_cast<bool>(digitalRead(INT)))

#ifdef SSPS3F1_BLACKOUT_EDITION
    #define CHANGE_KB_SIGNAL()          digitalWrite(INT_KB, !static_cast<bool>(digitalRead(INT_KB)))
#endif

String kbMsg = "";

/* I/O defines and arr var's based on defines */
#ifdef SSPS3F1_V1
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
#endif

#ifdef SSPS3F1_BLACKOUT_EDITION
    #define STM_RELAY_8     PC4
    #define STM_RELAY_7     PC5
    #define STM_RELAY_6     PB0
    #define STM_RELAY_5     PB1
    #define STM_RELAY_4     PB13
    #define STM_RELAY_3     PB14
    #define STM_RELAY_2     PB15
    #define STM_RELAY_1     PC6

    #define STM_DAC_1       PA4

    #define STM_OPT_IN_1    PB5
    #define STM_OPT_IN_2    PB4
    #define STM_OPT_IN_3    PB3
    #define STM_OPT_IN_4    PD2
    #define STM_OPT_IN_5    PC12
    #define STM_OPT_IN_6    PC11
    #define STM_OPT_IN_7    PC10
    #define STM_OPT_IN_8    PA15

    #define STM_ADC_1       PC3
    #define STM_ADC_2       PC2
    #define STM_ADC_3       PC1
    #define STM_ADC_4       PC0
#endif

uint8_t Relay[8]
{
    STM_RELAY_8,
    STM_RELAY_7,
    STM_RELAY_6,
    STM_RELAY_5,
    STM_RELAY_4,
    STM_RELAY_3,
    STM_RELAY_2,
    STM_RELAY_1
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

/* small extension for calc */
uint8_t InRange(uint8_t value, uint8_t max) {
    return value >= max ? max - 1 : (value < 0 ? 0 : value);
}

int getposition(const char *array, size_t size, char c)
{
     const char* end = array + size;
     const char* match = std::find(array, end, c);
     return end == match ? KB_Await : match-array;
}