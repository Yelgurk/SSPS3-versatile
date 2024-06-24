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

void setup()
{
    Serial.begin(115200);

    pinMode(INT, INPUT);
    attachInterrupt(INT, [](){ interrupted_by_slave = true; }, RISING);
    itcw = new TwoWire(0);
    itcw->begin(SDA, SCL, 400000);
    STM32 = new STM32_slave(STM_I2C_ADDR);

    UI_service.init();

    _cont = new UIClockx
    (
        {EquipmentType::All},
        {
            KeyModel(KeyMap::TOP,       []() { _cont->navi_prev(); }),
            KeyModel(KeyMap::BOTTOM,    []() { _cont->navi_next(); }),
            KeyModel(KeyMap::LEFT_TOP,  []() { _cont->navi_back(); }),
            KeyModel(KeyMap::LEFT_BOT,  []() { _cont->navi_ok(); }),
            KeyModel(KeyMap::R_STACK_4, []() { Serial.println("уровень 0"); })
        },
        PlaceControlIn::Screen,
        UI_service.screen
    );

    UI_clock = new UIClock
    (
        {EquipmentType::All},
        {
            /*
            KeyModel(KeyMap::LEFT,      []() { Serial.println("Дата/время влево"); }),
            KeyModel(KeyMap::TOP,       []() { Serial.println("Дата/время вверх"); }),
            KeyModel(KeyMap::RIGHT,     []() { Serial.println("Дата/время вправо"); }),
            KeyModel(KeyMap::BOTTOM,    []() { Serial.println("Дата/время вниз"); }),
            KeyModel(KeyMap::LEFT_TOP,  []() { UI_clock->lv_clear_states(); }),
            KeyModel(KeyMap::LEFT_BOT,  []() { UI_clock->lv_set_focused(true); })
            */
        },
        false,
        false,
        false,
        PlaceControlIn::Control,
        UI_service.screen,
        _cont
    );

    _list = new UIClock1
    (
        {EquipmentType::All},
        {
            KeyModel(KeyMap::TOP,       []() { _list->navi_prev(); }),
            KeyModel(KeyMap::BOTTOM,    []() { _list->navi_next(); }),
            KeyModel(KeyMap::LEFT_TOP,  []() { _list->navi_back(); }),
            KeyModel(KeyMap::LEFT_BOT,  []() { _list->navi_ok(); }),
            KeyModel(KeyMap::R_STACK_4, []() { Serial.println("уровень 1"); })
        },
        true, true, false,
        PlaceControlIn::Control,
        UI_service.screen,
        _cont
    );
    _list->load_list(&demo_task);

    UI_clock->add_ui_context_action([]() {
        lv_label_set_text(UI_clock->get_container_content("[time]"), to_string(++ss_ss).c_str());
    });
}

uint32_t ms_old = 0,
         ms_curr = 0;

void loop()
{
    lv_task_handler();

    if (interrupted_by_slave)
    {
        uint8_t key = STM32->get_kb();

        interrupted_by_slave = false;
        
        _cont->get_selected()->key_press(key);
        
        if(_cont->get_selected(true) != nullptr)
            _cont->get_selected(true)
            ->update_ui_base()
            ->update_ui_context();

        //UI_clock->key_press(key);
        //UI_clock->update_ui_context();
    }

    if ((ms_curr = millis()) - ms_old >= 1000)
    {
        Serial.println(millis()/1000);
        ms_old = millis();
        UI_clock->update_ui_context();
    }
}