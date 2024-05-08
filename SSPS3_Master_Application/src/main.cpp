#include "../include/main.hpp"

uint8_t FRAM_address = FRAM_I2C_ADDR;
TwoWire * itcw;
STM32_slave * STM32;

void setup()
{
    Serial.begin(115200);

    pinMode(INT, INPUT);
    attachInterrupt(INT, [](){ interrupted_by_slave = true; }, RISING);

    itcw = new TwoWire(0);
    itcw->begin(SDA, SCL, 400000);

    STM32 = new STM32_slave(STM_I2C_ADDR);
}


void loop()
{
    
}