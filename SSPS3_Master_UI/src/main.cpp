#include "../include/main.hpp"

uint32_t ss_ss = 0;
TwoWire * itcw;
STM32_slave * STM32;

vector<TaskData> demo_task
    {
        TaskData("zamesh", 10, 40, 600),
        TaskData("vymesh", 5, 50, 1000),
        TaskData("gooool", 15, 60, 2000)
    };

void IRAM_ATTR interrupt_action() {
    interrupted_by_slave = true;
}

void setup()
{
    Serial.begin(115200);

    itcw = new TwoWire(0);
    itcw->begin(SDA, SCL, 400000);
    STM32 = new STM32_slave(STM_I2C_ADDR);

    pinMode(INT, INPUT_PULLDOWN);
    attachInterrupt(digitalPinToInterrupt(INT), interrupt_action, CHANGE);

    UI_service.init();
}

void loop()
{
    lv_task_handler();

    if (interrupted_by_slave)
    {
        interrupted_by_slave = false;
       // _cont->get_selected()->key_press(STM32->get_kb());
    }
}