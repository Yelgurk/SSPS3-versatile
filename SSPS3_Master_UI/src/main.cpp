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
    UI_demo_screen = new UIScreen(UI_service.screen,
    {
        KeyModel(KeyMap::LEFT,      []() { Serial.println("Меню влево"); }),
        KeyModel(KeyMap::TOP,       []() { Serial.println("Меню вверх"); }),
        KeyModel(KeyMap::RIGHT,     []() { Serial.println("Меню вправо"); }),
        KeyModel(KeyMap::BOTTOM,    []() { Serial.println("Меню вниз"); }),
        KeyModel(KeyMap::LEFT_BOT,  []() { UI_demo_screen->focus("[ui_date_time]"); })
    });

    UI_clock = new UIClock(UI_demo_screen,
    {
        KeyModel(KeyMap::LEFT,      []() { Serial.println("Дата/время влево"); }),
        KeyModel(KeyMap::TOP,       []() { Serial.println("Дата/время вверх"); }),
        KeyModel(KeyMap::RIGHT,     []() { Serial.println("Дата/время вправо"); }),
        KeyModel(KeyMap::BOTTOM,    []() { Serial.println("Дата/время вниз"); }),
        KeyModel(KeyMap::LEFT_TOP,  []() { UI_clock->get_parent()->unfocus(); })
    });
    UI_clock->add_update_context_action([]() {
        lv_label_set_text(UI_clock->get_childs()->find("[time]")->second, to_string(++ss_ss).c_str());
    });

    UI_demo_screen->save_ui_element("[ui_date_time]", UI_clock);
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
        
        UI_demo_screen->key_press(key);
        UI_clock->update_context();
    }

    if ((ms_curr = millis()) - ms_old >= 1000)
    {
        ms_old = millis();
        UI_clock->update_context();
    }
}