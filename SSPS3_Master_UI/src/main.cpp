#include "../include/main.hpp"

#define SSPS_PLATFORMIO_INI_INFRASTRUCTURE  1
#define SSPS_STATE_BAR      1
#define SSPS_SCREEN_TASK    0
#define SSPS_MENU_USER      0
#define SSPS_BLOWING_PANEL  1

uint32_t ss_ss = 0;
TwoWire * itcw;
STM32_slave * STM32;

/* DEMO STRUCT BEGIN */
struct DEMO_BLOW_VAR
{
    uint16_t val = 5000;
    bool is_timer = false;

    DEMO_BLOW_VAR() {}
    DEMO_BLOW_VAR(uint16_t val) : val(val) {}
    DEMO_BLOW_VAR(bool is_timer, uint16_t val) : is_timer(is_timer), val(val) {}
};

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

    //DEMO_TASK(string name, vector<DEMO_TASK_STEP> * steps)
    //: name(name), steps(steps)
    //{}

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

        return aim_ss <= 0 ? 0 : 100.0 / (double)aim_ss * done_ss;
    }

    bool is_last_step(uint16_t index) {
        return index == steps->size() - 1;
    }

    int64_t seconds() {
        return millis() / 1000;
    }

    DEMO_TASK_STEP * start_task(string name, vector<DEMO_TASK_STEP> * steps)
    {
        if (state == TaskStateEnum::AWAIT || state == TaskStateEnum::DONE)
        {
            this->name = name;
            this->steps = steps;

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

/* реализация класса раздачи жидкости */
class BlowgunController
{
public:
    bool timer_running;
    bool is_active;
    using CallbackFunc = std::function<void(float, float, BlowingType, float)>;

private:
    DEMO_BLOW_VAR current_task;
    CallbackFunc callback;
    int8_t current_blow_index = -1;
    uint32_t ms_aim = 0;
    uint32_t ms_gone = 0;
    float ml_per_ms = 0;
    float pump_power_lm = 52.f;
    bool trigger_must_be_reloaded = false;
    bool pump_on;
    uint32_t last_call_time;
    uint32_t last_blow_time;

    void start_pump()
    {
        pump_on = true;
    }

    void stop_pump()
    {
        pump_on = false;
    }

public:
    BlowgunController(CallbackFunc callback) :
    callback(callback),
    is_active(false),
    last_call_time(0), 
    pump_on(false),
    timer_running(false)
    {}

    void blowgun_trigger(bool do_gurgling, bool is_keypad_press, int8_t index = -1, DEMO_BLOW_VAR curr_value = DEMO_BLOW_VAR())
    {
        if (!do_gurgling)
            trigger_must_be_reloaded = false;

        uint32_t current_time = millis();

        if (do_gurgling && !trigger_must_be_reloaded)
        {
            if (is_active && current_blow_index != index)
            {
                stop_pump();
                timer_running = false;
                return;
            }

            if (!is_active && is_keypad_press)
                callback(curr_value.val * 1000, 0.f, curr_value.is_timer ? BlowingType::TIMER : BlowingType::LITER, 0.001f);

            if ((!timer_running || !is_active) && ((is_keypad_press && current_time - last_call_time < 2000) || !is_keypad_press))
            {
                if (!is_active)
                {
                    current_task = curr_value;
                    current_blow_index = index;
                    is_active = true;

                    ml_per_ms = pump_power_lm * 1000.f / 60.f / 1000.f;

                    if (!current_task.is_timer)
                        ms_aim = (curr_value.val / (pump_power_lm * 1000)) * (60 * 1000);
                    else
                        ms_aim = curr_value.val * 1000;
                    ms_gone = 0;
                }

                last_blow_time = current_time;
                timer_running = true;
                start_pump();
            } 

            last_call_time = current_time;
        }
        else
        {
            stop_pump();
            timer_running = false;
        }
    }

    void blowgun_stop(bool need_in_reload = false)
    {
        stop_pump();
        if (need_in_reload)
            trigger_must_be_reloaded = true;
        timer_running = false;
        is_active = false;
        callback(0, 0, BlowingType::LITER, 0);
    }

    void do_blowing()
    {
        if (!is_active) return;

        uint32_t current_time = millis();
        if (current_time - last_call_time > 2000)
        {
            blowgun_stop();
            return;
        }

        if (timer_running)
        {
            ms_gone += current_time - last_blow_time;
            last_blow_time = current_time;
            callback(ms_aim, ms_gone, current_task.is_timer ? BlowingType::TIMER : BlowingType::LITER, ml_per_ms);

            if (ms_gone >= ms_aim)
            {
                callback(ms_aim, ms_gone, current_task.is_timer ? BlowingType::TIMER : BlowingType::LITER, ml_per_ms);
                blowgun_stop(true);
            }
        }
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

vector<DEMO_TASK_STEP> my_demo_task_steps_2 =
{
    DEMO_TASK_STEP("Выдержка", 19, 45, 120),
    DEMO_TASK_STEP("Нагрев", 17, 40, 90),
    DEMO_TASK_STEP("Ожидание", 21, 50, 120),
    DEMO_TASK_STEP("Набор воды", 5, 10, 10),
    DEMO_TASK_STEP("Нагрев", 7, 15, 20),
    DEMO_TASK_STEP("Пастеризация", 9, 20, 30),
    DEMO_TASK_STEP("Выдержка", 11, 25, 40),
    DEMO_TASK_STEP("Охлаждение", 13, 30, 50),
    DEMO_TASK_STEP("Выдержка", 15, 35, 60)
};

DEMO_TASK my_demo_task = DEMO_TASK();//("Пастеризация", &my_demo_task_steps);

vector<DEMO_BLOW_VAR> b_vars = {
    DEMO_BLOW_VAR(5000),
    DEMO_BLOW_VAR(5000),
    DEMO_BLOW_VAR(5000),
    DEMO_BLOW_VAR(true, 360)
};

BlowgunController pumpController = BlowgunController(
    [](float ms_aim, float ms_gone, BlowingType type, float ml_per_ms) 
    {
        UI_blowing_control->set_blow_value(ms_aim, ms_gone, type, ml_per_ms);
    }
);
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

#if SSPS_PLATFORMIO_INI_INFRASTRUCTURE == 1
    STM32 = new STM32_slave(STM_I2C_ADDR);

    pinMode(INT, INPUT_PULLDOWN);
    attachInterrupt(digitalPinToInterrupt(INT), interrupt_action, CHANGE);
#endif

    UI_service.init();
    lcd.setBrightness(255);
    
    init_ui_controls();

#if SSPS_SCREEN_TASK == 1 
    my_demo_task.start_task("Пастеризация", &my_demo_task_steps);
    UI_task_roadmap_control->update_ui_base();
    UI_task_roadmap_control->update_ui_context();
#endif
}

void loop()
{
    lv_task_handler();

    #if SSPS_BLOWING_PANEL == 1
        pumpController.do_blowing();
    #endif

    if (millis() - ms_last_2 >= 250)
    {
        ms_last_2 = millis();
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
        if (my_demo_task.do_task() == nullptr)
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

        UI_task_roadmap_control->update_task_steps_state();
        UI_task_roadmap_control->update_ui_context();
#endif
    }

#if SSPS_PLATFORMIO_INI_INFRASTRUCTURE == 1
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

#endif
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
        []() { if (my_demo_task.state != TaskStateEnum::AWAIT) UI_task_roadmap_control->set_task_header_name(my_demo_task.name); }
    );
    UI_task_roadmap_control->add_ui_context_action(
        []()
        {
            if (my_demo_task.state != TaskStateEnum::AWAIT)
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
            KeyModel(KeyMap::LEFT_TOP, []()
            {
                if (pumpController.is_active)
                    pumpController.blowgun_stop();
                else
                    UI_blowing_control->navi_back();
            }),
            KeyModel(KeyMap::RIGHT_BOT_REL, []() { pumpController.blowgun_trigger(false, true); })
        },
        UI_service.screen
    );

    for (uint8_t i = 0; i < 4; i++)
    {
        Blow_vars.push_back(new UIBlowValListItem(UI_blowing_control, i < 3 ? BlowVarTypeEnum::LITRES : BlowVarTypeEnum::TIMER));
        UIBlowValListItem * blow_ptr = Blow_vars.back();

        blow_ptr->add_ui_context_action([=]() { blow_ptr->set_value(b_vars.at(i).val, i != 3 ? " л." : ""); });
        blow_ptr->set_extra_button_logic({
            [i]() { b_vars.at(i).val += (i != 3 ? 250 : 5); },
            [i]() { b_vars.at(i).val -= (i != 3 ? 250 : 5); },
            [i]() { pumpController.blowgun_trigger(true, true, i, b_vars.at(i)); }
        });
    }
#endif
}