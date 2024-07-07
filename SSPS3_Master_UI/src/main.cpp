#include "../include/main.hpp"

TwoWire         * itcw;
STM32_slave     * STM32;
FRAM_DB         * FRAM_db;
ProgramControl  * Program_control;
BlowingControl  * Blowing_control;
UIService       * UI_service; 

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

    STM32           = new STM32_slave(STM_I2C_ADDR);
    FRAM_db         = new FRAM_DB();
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
    }, 250);

    rt_task_manager.add_task("do_program_task", []() {
        Program_control->do_task();
        if (Program_control->is_active)
        {
            UI_service->UI_task_roadmap_control->update_task_steps_state();
            UI_service->UI_task_roadmap_control->update_ui_context();
        }
    }, 500);

    /*
    сделать обдумать, как загружать новые программы по примеру ниже

    if (!my_demo_task.is_active)
    {
        my_demo_task.start_task("Пастер 2", &my_demo_task_steps_2);

        for (uint16_t i = 0; i < my_demo_task_steps_2.size(); i++)
        {
            DEMO_TASK_STEP* step = &my_demo_task_steps_2.at(i);
    
            UITaskListItem* ui_step = UI_task_roadmap_control->add_task_step(i == 0);
            ui_step->set_extra_button_logic({
                [=](){ step->fan++;          },
                [=](){ step->fan--;          },
                [=](){ step->tempC++;        },
                [=](){ step->tempC--;        },
                [=](){ step->duration += 10; },
                [=](){ step->duration -= 10; },
            });
    
            ui_step->add_ui_base_action([ui_step, step]() { ui_step->set_step_name(step->name); });
            ui_step->add_ui_context_action([ui_step, step]() { ui_step->set_step_values(step->fan, step->tempC, step->time_left_ss(), step->state); });
        }
        UI_task_roadmap_control->update_ui_base();
        UI_task_roadmap_control->update_ui_context();
    }
    */
}

void loop()
{
    lv_task_handler();

    Blowing_control->do_blowing();
    rt_task_manager.run();

    if (interrupted_by_slave)
    {
        interrupted_by_slave = false;
        uint8_t x = STM32->get_kb();
        
        /*

        // task control
        UI_task_roadmap_control->get_selected()->key_press(x);
        UI_task_roadmap_control->get_selected(true)->key_press(x);

        // user settings control
        UI_menu_list_user->get_selected()->key_press(x);
        if (UI_menu_list_user->is_selected_on_child())
            UI_menu_list_user->get_selected(true)->key_press(x);

        // blowing control
        UI_blowing_control->get_selected()->key_press(x);
        UI_blowing_control->get_selected(true)->key_press(x);

        // notification control
        UI_notification_bar->key_press(x);
        
        */
    }
}