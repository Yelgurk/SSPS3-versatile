#include "../include/main.hpp"

uint32_t ss_ss = 0;
TwoWire * itcw;
STM32_slave * STM32;

vector<UITaskItemData> list;

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
    
    lcd.setBrightness(255);

    init_ui_controls();
    

/* TASK LIST BEGIN */
    list.clear();

    for (uint32_t i = 0; i < 10; i++)
    {
        uint32_t fan = random(0,30),
             tempc = random(10, 85),
             durat = random(10, 1000);
        list.push_back(UITaskItemData(to_string(i).c_str(), fan, tempc, durat));
    }

    UI_task_roadmap_control->load_task_list(&list);
/* TASK LSIT END */
}

uint32_t ms_last = 0;
uint32_t counter = 0;

void loop()
{
    lv_task_handler();
    
    if (millis() - ms_last >= 1000)
    {
        
    }

    if (interrupted_by_slave)
    {
        interrupted_by_slave = false;
        uint8_t x = STM32->get_kb();

        UI_task_roadmap_control->get_selected()->key_press(x);
        UI_task_roadmap_control->get_selected(true)->key_press(x);
    }
}

void init_ui_controls()
{
    UI_task_roadmap_control = new UITaskRoadmapList(
        {
            KeyModel(KeyMap::TOP, []() { UI_task_roadmap_control->navi_prev(); }),
            KeyModel(KeyMap::BOTTOM, []() { UI_task_roadmap_control->navi_next(); })
        },
        UI_service.screen
    );

    UI_date_time = new UIDateTime(UI_service.screen);

    UI_machine_state_bar = new UIMachineStateBar(UI_service.screen);
}