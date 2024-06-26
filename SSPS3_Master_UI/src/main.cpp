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
    STM32->startup();

    pinMode(INT, INPUT_PULLDOWN);
    attachInterrupt(digitalPinToInterrupt(INT), interrupt_action, RISING);

    UI_service.init();
}

uint32_t ms_old = 0,
         ms_curr = 0,
         ms_old_2 = 0,
         ms_curr_2 = 0;

void loop()
{
    lv_task_handler();

    if ((ms_curr = millis()) - ms_old >= 1000)
    {
        STM32->ping();
        ms_old = millis();
    }

    if ((ms_curr_2 = millis()) - ms_old_2 >= 100)
    {
        print();
        ms_old_2 = millis();
    }

    if (interrupted_by_slave)
    {
        interrupted_by_slave = false;
        print(true);
       // _cont->get_selected()->key_press(STM32->get_kb());
    }
}

void print(bool only_digital)
{
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
    
    if (!only_digital)
    {
        Serial.print(" | ");

        Serial.print(STM32->get(COMM_GET::ANIN, 0));
        Serial.print(" ");
        Serial.print(STM32->get(COMM_GET::ANIN, 1));
        Serial.print(" ");
        Serial.print(STM32->get(COMM_GET::ANIN, 2));
        Serial.print(" ");
        Serial.println(STM32->get(COMM_GET::ANIN, 3));
    }
    else
        Serial.println();
}