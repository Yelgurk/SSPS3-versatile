#include "../include/main.hpp"

TwoWire         * itcw;
DS3231          * rtc;
STM32_slave     * STM32;
ProgramControl  * Program_control;
BlowingControl  * Blowing_control;
UIService       * UI_service; 
S_DateTime      * dt_rt;

void setup()
{
    Serial.begin(115200);

    if (!psramFound())
        Serial.println("PSRAM not found");
    else
        Serial.println("PSRAM found and initialized");

    pinMode(INT, INPUT_PULLDOWN);
    attachInterrupt(digitalPinToInterrupt(INT), interrupt_action, CHANGE);

    itcw = new TwoWire(0);
    itcw->begin(SDA, SCL, 400000);

    rtc             = new DS3231(*itcw);
    dt_rt = new S_DateTime(0, 0, 0, 0, 0, 0, []() {
        return make_tuple(
            (uint8_t)rtc->getHour(h12Flag, pmFlag),
            (uint8_t)rtc->getMinute(),
            (uint8_t)rtc->getSecond(),
            (uint8_t)rtc->getDate(),
            (uint8_t)rtc->getMonth(century),
            (uint8_t)rtc->getYear()
        );
    });
    dt_rt->get_rt();

    STM32           = new STM32_slave(STM_I2C_ADDR);
    Program_control = new ProgramControl();
    Blowing_control = new BlowingControl();
    UI_service      = new UIService();

    Blowing_control->set_blowing_ui_callback(
        [](float ms_aim, float ms_gone, BlowingType type, float ml_per_ms) {
            UI_service->UI_blowing_control->set_blow_value(ms_aim, ms_gone, type, ml_per_ms);
        }
    );

    rt_task_manager.add_task("update_state_bar_task", [](){
        UI_service->UI_machine_state_bar->control_set_values_state_bar(
            random(0, 31),
            random(10, 86),
            static_cast<WaterJacketStateEnum>(random(0, 3)),
            random(0, 101),
            ChargeStateEnum::STABLE
        );
        
        dt_rt->get_rt();
        UI_service->UI_date_time->control_set_values_date_time(
            dt_rt->get_time()->get_hours(),
            dt_rt->get_time()->get_minutes(),
            dt_rt->get_time()->get_seconds(),
            dt_rt->get_date()->get_day(),
            dt_rt->get_date()->get_month(),
            dt_rt->get_date()->get_year()
        );
    }, 500);

    rt_task_manager.add_task("do_program_task", []() {
        Program_control->do_task();
        UI_service->UI_task_roadmap_control->update_task_steps_state();
        UI_service->UI_task_roadmap_control->update_ui_context();
    }, 500);
}

void loop()
{
    Blowing_control->do_blowing();
    rt_task_manager.run();

    if (interrupted_by_slave)
    {
        interrupted_by_slave = false;
        read_input_signals();

        UI_service->UI_notification_bar->key_press(Pressed_key);
        
        // task control
        //UI_service->UI_task_roadmap_control->get_selected()->key_press(Pressed_key);
        //UI_service->UI_task_roadmap_control->get_selected(true)->key_press(Pressed_key);

        // user settings control
        UI_service->UI_menu_list_user->get_selected()->key_press(Pressed_key);
        if (UI_service->UI_menu_list_user->is_selected_on_child())
            UI_service->UI_menu_list_user->get_selected(true)->key_press(Pressed_key);

        // blowing control
        //UI_service->UI_blowing_control->get_selected()->key_press(Pressed_key);
        //UI_service->UI_blowing_control->get_selected(true)->key_press(Pressed_key);
    }

    lv_task_handler();
}