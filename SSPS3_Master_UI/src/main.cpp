#include "../include/main.hpp"

#define SSPS_STATE_BAR      1
#define SSPS_SCREEN_TASK    0
#define SSPS_MENU_USER      1
#define SSPS_BLOWING_PANEL  0

uint32_t ss_ss = 0;
TwoWire * itcw;
STM32_slave * STM32;

/* DEMO STRUCT BEGIN */
struct DEMO_TASK_STEP
{
    string name;
    uint8_t fan,
            tempC;
    uint32_t duration;
    uint32_t gone_ss = 0;
    StepStateEnum state = StepStateEnum::AWAIT;
    bool await_ok_button = false;

    DEMO_TASK_STEP(string name, uint8_t fan, uint8_t tempC, uint32_t duration)
    : name(name), fan(fan), tempC(tempC), duration(duration)
    {}

    bool is_time_out() {
        return gone_ss >= duration;
    }

    void iteration_ss(uint32_t ss) {
        gone_ss += ss;
    }

    int32_t time_left_ss() {
        return duration - gone_ss;
    }
};

struct DEMO_TASK
{
    string name;

    int64_t started_at_ss;
    int64_t last_iteration_ss;
    int64_t gone_ss;
    TaskStateEnum state = TaskStateEnum::AWAIT;
    vector<DEMO_TASK_STEP> * steps;

    DEMO_TASK(string name, vector<DEMO_TASK_STEP> * steps)
    : name(name), steps(steps)
    {}

    double get_prog_percentage()
    {
        int32_t aim_ss = 0,
                done_ss = 0;

        for (auto step : *steps)
        {
            aim_ss += step.duration;

            if (step.state == StepStateEnum::RUNNED || step.state == StepStateEnum::DONE)
                done_ss += (step.gone_ss > step.duration ? step.duration : step.gone_ss);
        }

        return 100.0 / (double)aim_ss * done_ss;
    }

    bool is_last_step(uint16_t index) {
        return index == steps->size() - 1;
    }

    int64_t seconds() {
        return millis() / 1000;
    }

    DEMO_TASK_STEP * start_task()
    {
        if (state == TaskStateEnum::AWAIT)
        {
            started_at_ss = seconds();
            last_iteration_ss = seconds();
            gone_ss = 0;
            state = TaskStateEnum::RUNNED;
        }

        return do_task();
    }

    /* return current step */
    DEMO_TASK_STEP * do_task()
    {
        int32_t ss_from_last_iteration = seconds() - last_iteration_ss;
        last_iteration_ss = seconds();
    
        if (state == TaskStateEnum::RUNNED)
        {
            gone_ss = seconds() - started_at_ss;

            for (int i = 0; i < steps->size(); i++)
            {
                if (steps->at(i).state == StepStateEnum::AWAIT && i == 0)
                {
                    steps->at(i).iteration_ss(ss_from_last_iteration);
                    steps->at(i).state = StepStateEnum::RUNNED;
                    return &steps->at(i);
                }

                if (steps->at(i).state == StepStateEnum::RUNNED)
                {
                    steps->at(i).iteration_ss(ss_from_last_iteration);

                    if (steps->at(i).is_time_out())
                    {
                        steps->at(i).state = StepStateEnum::DONE;

                        if (!is_last_step(i))
                        {
                            steps->at(i + 1).state = StepStateEnum::RUNNED;
                            return &steps->at(i + 1);
                        }
                        else
                        {
                            state = TaskStateEnum::DONE;
                            return &steps->at(i);
                        }
                    }
                    else
                        return &steps->at(i);
                }
            }
        }

        return nullptr;
    }
};
/* DEMO STRUCT END */

/* DEMO VARS BEGIN*/
uint32_t ms_last = 0,
         ms_last_2 = 0;
int32_t counter = 0;
bool demo_flag = false;

vector<DEMO_TASK_STEP> my_demo_task_steps =
{
    DEMO_TASK_STEP("Набор воды", 5, 10, 10),
    DEMO_TASK_STEP("Нагрев", 7, 15, 20),
    DEMO_TASK_STEP("Пастеризация", 9, 20, 30),
    DEMO_TASK_STEP("Выдержка", 11, 25, 40),
    DEMO_TASK_STEP("Охлаждение", 13, 30, 50),
    DEMO_TASK_STEP("Выдержка", 15, 35, 60),
    DEMO_TASK_STEP("Нагрев", 17, 40, 90),
    DEMO_TASK_STEP("Выдержка", 19, 45, 120),
    DEMO_TASK_STEP("Ожидание", 21, 50, 120)
};

DEMO_TASK my_demo_task("Пастеризация", &my_demo_task_steps);
/* DEMO VARS END */

void init_ui_controls();

void IRAM_ATTR interrupt_action() {
    interrupted_by_slave = true;
}

void setup()
{
    Serial.begin(115200);

    if (!psramFound())
        Serial.println("PSRAM not found");
    else
        Serial.println("PSRAM found and initialized");

    itcw = new TwoWire(0);
    itcw->begin(SDA, SCL, 400000);
    STM32 = new STM32_slave(STM_I2C_ADDR);

    pinMode(INT, INPUT_PULLDOWN);
    attachInterrupt(digitalPinToInterrupt(INT), interrupt_action, CHANGE);

    UI_service.init();
    lcd.setBrightness(255);
    
    init_ui_controls();

    my_demo_task.start_task();
}

void loop()
{
    lv_task_handler();

    if (millis() - ms_last_2 >= 5)
    {

        if (--counter <= 0)
        {
            counter = 5000;
            demo_flag = !demo_flag;
        }

#if SSPS_BLOWING_PANEL == 1
        UI_blowing_control->set_blow_value(
            5000,
            counter,
            demo_flag ? BlowingType::LITER : BlowingType::TIMER
        );
#endif
    }

    if (millis() - ms_last >= 1000)
    {
        ms_last = millis();

#if SSPS_STATE_BAR == 1
        UI_machine_state_bar->control_set_values_state_bar(
            random(0, 31),
            random(10, 86),
            static_cast<WaterJacketStateEnum>(random(0, 3)),
            random(0, 101),
            ChargeStateEnum::STABLE
        );
#endif
    
#if SSPS_SCREEN_TASK == 1
        my_demo_task.do_task();
        UI_task_roadmap_control->update_task_steps_state();
        UI_task_roadmap_control->update_ui_context();
#endif
    }

    if (interrupted_by_slave)
    {
        interrupted_by_slave = false;
        uint8_t x = STM32->get_kb();

#if SSPS_SCREEN_TASK == 1
        UI_task_roadmap_control->get_selected()->key_press(x);
        UI_task_roadmap_control->get_selected(true)->key_press(x);
#endif

#if SSPS_MENU_USER == 1
        UI_menu_list_user->get_selected()->key_press(x);
#endif

#if SSPS_BLOWING_PANEL == 1
        UI_blowing_control->get_selected()->key_press(x);
        UI_blowing_control->get_selected(true)->key_press(x);
#endif
    }
}

void init_ui_controls()
{
#if SSPS_STATE_BAR == 1
    UI_date_time = new UIDateTime(UI_service.screen);
    UI_machine_state_bar = new UIMachineStateBar(UI_service.screen);
    UI_notify_bar = new UINotifyBar(UI_service.screen);
#endif

#if SSPS_SCREEN_TASK == 1
    UI_task_roadmap_control = new UITaskRoadmapList(
        {
            KeyModel(KeyMap::TOP, []()
            {
                UI_task_roadmap_control->navi_prev();
                UI_task_roadmap_control->update_task_steps_state();
            }),
            KeyModel(KeyMap::BOTTOM, []()
            {
                UI_task_roadmap_control->navi_next();
                UI_task_roadmap_control->update_task_steps_state();
            })
        },
        UI_service.screen
    );
    UI_task_roadmap_control->add_ui_base_action(
        []() { UI_task_roadmap_control->set_task_header_name(my_demo_task.name); }
    );
    UI_task_roadmap_control->add_ui_context_action(
        []()
        {
            UI_task_roadmap_control->set_task_state_values(
                my_demo_task.get_prog_percentage(),
                my_demo_task.gone_ss,
                my_demo_task.state
            );
        }
    );
        
    //set_extra_button_logic

    for (uint16_t i = 0; i < my_demo_task_steps.size(); i++)
    {
        DEMO_TASK_STEP* step = &my_demo_task_steps.at(i);

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
#endif

#if SSPS_MENU_USER == 1
    UI_menu_list_user = new UIMenuList(
        UI_service.screen,
        {
            KeyModel(KeyMap::TOP, []() { UI_menu_list_user->navi_prev(); }),
            KeyModel(KeyMap::BOTTOM, []() { UI_menu_list_user->navi_next(); }),
            KeyModel(KeyMap::LEFT_TOP,  []() { UI_menu_list_user->navi_back(); }),
            KeyModel(KeyMap::LEFT_BOT,  []() { UI_menu_list_user->navi_ok(); })
        }
    );

    UI_settings_user_datetime = new UIMenuListItem(UI_menu_list_user, "Привет");
    UI_settings_user_pump = new UIMenuListItem(UI_menu_list_user, "Как дела?");
    UI_settings_user_pasteurizer_template_1 = new UIMenuListItem(UI_menu_list_user, "У меня нормально, как-то так");
    
    UI_settings_user_pasteurizer_template_1->set_page_header("Установка времени", 0);
    UI_settings_user_pasteurizer_template_1->set_page_header("Установка времени 2", 1);

    UI_Set1 = new UIValueSetter(
        UI_settings_user_pasteurizer_template_1,
        LV_ALIGN_TOP_LEFT, 0,
        0, 10, 40, true,
        "", &img_fan
    );

    UI_Set2 = new UIValueSetter(
        UI_settings_user_pasteurizer_template_1,
        LV_ALIGN_TOP_LEFT, 1,
        40, 10, 40, false,
        "", &img_tempC
    );

    UI_Set3 = new UIValueSetter(
        UI_settings_user_pasteurizer_template_1,
        LV_ALIGN_TOP_LEFT, 1,
        40, 50, 40, false,
        "час"
    );
#endif

#if SSPS_BLOWING_PANEL == 1
    UI_blowing_control = new UIBlowingControl(
        {
            KeyModel(KeyMap::BOTTOM, []() { UI_blowing_control->navi_next(); }),
            KeyModel(KeyMap::TOP, []() { UI_blowing_control->navi_prev(); }),
            KeyModel(KeyMap::LEFT_TOP, []() { UI_blowing_control->navi_back(); })
        },
        UI_service.screen
    );

    Blow_var_1 = new UIBlowValListItem(UI_blowing_control, BlowVarTypeEnum::LITRES);
    Blow_var_2 = new UIBlowValListItem(UI_blowing_control, BlowVarTypeEnum::LITRES);
    Blow_var_3 = new UIBlowValListItem(UI_blowing_control, BlowVarTypeEnum::LITRES);
    Blow_var_timer = new UIBlowValListItem(UI_blowing_control, BlowVarTypeEnum::TIMER);
#endif
}