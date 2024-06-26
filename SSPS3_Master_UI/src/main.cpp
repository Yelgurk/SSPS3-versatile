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
        print();
       // _cont->get_selected()->key_press(STM32->get_kb());
    }
}

uint32_t cnttt = 0;

void print()
{
    Serial.print(++cnttt);

    Serial.print(" - ");

    Serial.print(STM32->get_kb());

    Serial.print(" | ");

    Serial.print(OptIn_state[0] = STM32->get(COMM_GET::DGIN, 0));
    Serial.print(" ");
    Serial.print(OptIn_state[1] = STM32->get(COMM_GET::DGIN, 1));
    Serial.print(" ");
    Serial.print(OptIn_state[2] = STM32->get(COMM_GET::DGIN, 2));
    Serial.print(" ");
    Serial.print(OptIn_state[3] = STM32->get(COMM_GET::DGIN, 3));
    Serial.print(" ");
    Serial.print(OptIn_state[4] = STM32->get(COMM_GET::DGIN, 4));
    Serial.print(" ");
    Serial.print(OptIn_state[5] = STM32->get(COMM_GET::DGIN, 5));
    Serial.print(" ");
    Serial.print(OptIn_state[6] = STM32->get(COMM_GET::DGIN, 6));
    Serial.print(" ");
    Serial.print(OptIn_state[7] = STM32->get(COMM_GET::DGIN, 7));
    
    Serial.print(" | ");

    Serial.print(STM32->get(COMM_GET::ANIN, 0));
    Serial.print(" ");
    Serial.print(STM32->get(COMM_GET::ANIN, 1));
    Serial.print(" ");
    Serial.print(STM32->get(COMM_GET::ANIN, 2));
    Serial.print(" ");
    Serial.println(STM32->get(COMM_GET::ANIN, 3)); 
}