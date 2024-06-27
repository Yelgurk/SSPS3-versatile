#include "../include/main.hpp"

uint32_t ss_ss = 0;
TwoWire * itcw;
STM32_slave * STM32;

void init_ui_controls();

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
    init_ui_controls();
}

uint32_t ms_last = 0;

void loop()
{
    lv_task_handler();

    if (millis() - ms_last >= 1000)
    {
        ms_last = millis();
        Serial.println(ESP.getFreeHeap());
    }

    if (interrupted_by_slave)
    {
        interrupted_by_slave = false;
       // _cont->get_selected()->key_press(STM32->get_kb());
    }
}

void init_ui_controls()
{
    UI_task_roadmap_control = new UITaskRoadmapList({}, UI_service.screen);
}