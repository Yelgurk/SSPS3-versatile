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
    init_ui_controls();
}

uint32_t ms_last = 0,
         ms_last_2 = 0,
         ms_last3 = 0;

uint32_t xxxx = 0;

void loop()
{
    lv_task_handler();
    
    if (millis() - ms_last3 >= 10000)
    {
        list.clear();

        uint32_t cnt = random(0, 15);

        for (uint32_t i = 0; i < cnt; i++)
        {
            uint32_t fan = random(0,30),
                 tempc = random(10, 85),
                 durat = random(10, 1000);

            list.push_back(UITaskItemData(to_string(i).c_str(), fan, tempc, durat));
        }

        UI_task_roadmap_control->load_task_list(&list);
        ms_last3 = millis();
    }

    if (millis() - ms_last >= 1000)
    {
        ms_last = millis();
        Serial.println(ESP.getFreeHeap());

        ++ss_ss;
    }

    if (millis() - ms_last_2 >= 200)
    {
        xxxx += 10000 / 200;
        ms_last_2 = millis();
        UI_task_roadmap_control->update_ui_context();
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

    UI_task_roadmap_control
    ->add_ui_context_action([]() {
        lv_obj_t * _progress_bar        = UI_task_roadmap_control->get_container_content("[progress_bar]");
        lv_obj_t * _label_percentage    = UI_task_roadmap_control->get_container_content("[state_percentage]");
        lv_obj_t * _label_duration      = UI_task_roadmap_control->get_container_content("[state_duration]");

        lv_label_set_text(_label_percentage, to_string(xxxx / 100 % 101).c_str());
        lv_label_set_text(_label_duration, to_string(ss_ss).c_str());
        lv_arc_set_value(_progress_bar, xxxx % 10100);
    });
}