#include "../include/main.hpp"

uint32_t ss_ss = 0;
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
        true,
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
        true,
        false,
        PlaceControlIn::Control,
        UI_service.screen,
        _cont
    );

    _c1 = new UIClock2
    (
        {EquipmentType::All},
        {
            KeyModel(KeyMap::TOP,       []() { _c1->navi_prev(); }),
            KeyModel(KeyMap::BOTTOM,    []() { _c1->navi_next(); }),
            KeyModel(KeyMap::LEFT_TOP,  []() { _c1->navi_back(); }),
            KeyModel(KeyMap::LEFT_BOT,  []() { _c1->navi_ok(); }),
            KeyModel(KeyMap::R_STACK_4, []() { Serial.println("уровень 2"); })
        },
        true,
        false,
        PlaceControlIn::Control,
        UI_service.screen,
        _list
    );

    _c2 = new UIClock3
    (
        {EquipmentType::All},
        {
            KeyModel(KeyMap::TOP,       []() { _c2->navi_prev(); }),
            KeyModel(KeyMap::BOTTOM,    []() { _c2->navi_next(); }),
            KeyModel(KeyMap::LEFT_TOP,  []() { _c2->navi_back(); }),
            KeyModel(KeyMap::LEFT_BOT,  []() { _c2->navi_ok(); }),
            KeyModel(KeyMap::R_STACK_4, []() { Serial.println("уровень 2"); })
        },
        true,
        false,
        PlaceControlIn::Control,
        UI_service.screen,
        _list
    );

    _c3 = new UIClock4
    (
        {EquipmentType::All},
        {
            KeyModel(KeyMap::TOP,       []() { _c3->navi_prev(); }),
            KeyModel(KeyMap::BOTTOM,    []() { _c3->navi_next(); }),
            KeyModel(KeyMap::LEFT_TOP,  []() { _c3->navi_back(); }),
            KeyModel(KeyMap::LEFT_BOT,  []() { _c3->navi_ok(); }),
            KeyModel(KeyMap::R_STACK_4, []() { Serial.println("уровень 2"); })
        },
        true,
        false,
        PlaceControlIn::Control,
        UI_service.screen,
        _list
    );

    _c1_1 = new UIClock5
    (
        {EquipmentType::All},
        {
            KeyModel(KeyMap::TOP,       []() { _c1_1->navi_prev(); }),
            KeyModel(KeyMap::BOTTOM,    []() { _c1_1->navi_next(); }),
            KeyModel(KeyMap::LEFT_TOP,  []() { _c1_1->navi_back(); }),
            KeyModel(KeyMap::LEFT_BOT,  []() { _c1_1->navi_ok(); }),
            KeyModel(KeyMap::R_STACK_4, []() { Serial.println("уровень 3"); })
        },
        true,
        false,
        PlaceControlIn::Screen,
        UI_service.screen,
        _c1
    );

    _c1_2 = new UIClock6
    (
        {EquipmentType::All},
        {
            KeyModel(KeyMap::TOP,       []() { _c1_2->navi_prev(); }),
            KeyModel(KeyMap::BOTTOM,    []() { _c1_2->navi_next(); }),
            KeyModel(KeyMap::LEFT_TOP,  []() { _c1_2->navi_back(); }),
            KeyModel(KeyMap::LEFT_BOT,  []() { _c1_2->navi_ok(); }),
            KeyModel(KeyMap::R_STACK_4, []() { Serial.println("уровень 3"); })
        },
        true,
        false,
        PlaceControlIn::Screen,
        UI_service.screen,
        _c1
    );

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
        //UI_clock->key_press(key);
        //UI_clock->update_ui_context();
    }

    if ((ms_curr = millis()) - ms_old >= 1000)
    {
        ms_old = millis();
        UI_clock->update_ui_context();
    }
}