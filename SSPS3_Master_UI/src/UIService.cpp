#include "../include/UIService.hpp"

UIService::UIService()
{
    lcd.init();
    lcd.setRotation(0);
    lcd.setBrightness(255);

    lv_init();
    lv_tick_set_cb(arduino_tick_get_cb);

    lv_display_t* disp = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(
        disp,
        [](lv_display_t* display, const lv_area_t* area, unsigned char* data) {
            lcd_flush_cb(display, area, data);
        }
    );

#if INIT_BUFFER_IN_PSRAM == 0
    lv_display_set_buffers(disp, lv_buff_1, lv_buff_2, SCREEN_BUFFER, LV_DISPLAY_RENDER_MODE_PARTIAL);
#else
    lv_buff_1 = new PSRAMBuffer(SCREEN_BUFFER);
    lv_buff_2 = new PSRAMBuffer(SCREEN_BUFFER);
    lv_display_set_buffers(disp, lv_buff_1->getBuffer(), lv_buff_2->getBuffer(), SCREEN_BUFFER, LV_DISPLAY_RENDER_MODE_PARTIAL);
#endif
    
    lv_disp_set_default(disp);

    screen = lv_obj_create(NULL);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(screen, COLOR_WHITE_SMOKE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_width(screen, SCREEN_WIDTH);
    lv_obj_set_height(screen, SCREEN_HEIGHT);

    lv_disp_load_scr(screen);
}

void UIService::init_screens()
{
    /* upper state bar init */
    UI_date_time = new UIDateTime(this->screen);
    UI_machine_state_bar = new UIMachineStateBar(this->screen);
    UI_notify_bar = new UINotifyBar(this->screen);
    UI_notification_bar = new UINotificationBar(this->screen);

    /* task roadmap controls init */
    UI_task_roadmap_control = new UITaskRoadmapList(
        {
            KeyModel(KeyMap::TOP, [this]()
            {
                UI_task_roadmap_control->navi_prev();
                UI_task_roadmap_control->update_task_steps_state();
            }),
            KeyModel(KeyMap::BOTTOM, [this]()
            {
                UI_task_roadmap_control->navi_next();
                UI_task_roadmap_control->update_task_steps_state();
            }),
            KeyModel(KeyMap::RIGHT_BOT, [this]()
            {
                my_demo_task.pause_task();
                UI_task_roadmap_control->update_ui_context();
                UI_task_roadmap_control->update_task_steps_state();
            }),
            KeyModel(KeyMap::L_STACK_1, [this]()
            {
                my_demo_task.end_task();
                UI_task_roadmap_control->update_ui_context();
                UI_task_roadmap_control->update_task_steps_state();
            })
        },
        this->screen
    );

    UI_task_roadmap_control->add_ui_base_action(
        [this]() { if (my_demo_task.state != TaskStateEnum::AWAIT) UI_task_roadmap_control->set_task_header_name(my_demo_task.name); }
    );

    UI_task_roadmap_control->add_ui_context_action(
        [this]()
        {
            if (my_demo_task.state != TaskStateEnum::AWAIT)
                UI_task_roadmap_control->set_task_state_values(
                    my_demo_task.get_prog_percentage(),
                    my_demo_task.gone_ss,
                    my_demo_task.state
                );
        }
    );

    /*
    *
    *
    *
    *
    * 
    * 
    * 
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
    * 
    * 
    * 
    * 
    * 
    * 
    * 
    * 
    */

    /* blowing panel init */
    UI_blowing_control = new UIBlowingControl(
        {
            KeyModel(KeyMap::BOTTOM, [this]() { UI_blowing_control->navi_next(); }),
            KeyModel(KeyMap::TOP, [this]() { UI_blowing_control->navi_prev(); }),
            KeyModel(KeyMap::LEFT_TOP, [this]()
            {
                if (pumpController.is_active)
                    pumpController.blowgun_stop();
                else
                    UI_blowing_control->navi_back();
            }),
            KeyModel(KeyMap::RIGHT_BOT_REL, [this]() { pumpController.blowgun_trigger(false, true); })
        },
        this->screen
    );
    init_blowing_controls();

    /* user menu panel init */
    *demo_setter_value = 0;

    UI_menu_list_user = new UIMenuList(
        this->screen,
        {
            KeyModel(KeyMap::TOP, [this]() { UI_menu_list_user->navi_prev(); }),
            KeyModel(KeyMap::BOTTOM, [this]() { UI_menu_list_user->navi_next(); }),
            KeyModel(KeyMap::LEFT_TOP,  [this]() { UI_menu_list_user->navi_back(); }),
            KeyModel(KeyMap::LEFT_BOT,  [this]() { UI_menu_list_user->navi_ok(); })
        }
    );
    init_settings_user_controls();
}

void UIService::init_blowing_controls()
{
    /*
    b_vars - были тестовые данные для обкатки контрола раздачи. будет во FRAM 
    
    vector<DEMO_BLOW_VAR> b_vars = {
    DEMO_BLOW_VAR(5000),
    DEMO_BLOW_VAR(5000),
    DEMO_BLOW_VAR(5000),
    DEMO_BLOW_VAR(true, 360)
    };
    */

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
}

void UIService::init_settings_user_controls()
{
    UI_settings_user_datetime = new UIMenuListItem(UI_menu_list_user, "Привет");
    UI_settings_user_pump = new UIMenuListItem(UI_menu_list_user, "Как дела?");
    UI_settings_user_pasteurizer_template_1 = new UIMenuListItem(UI_menu_list_user, "У меня нормально, как-то так");
    
    UI_settings_user_pasteurizer_template_1->set_page_header("Установка времени", 0);
    UI_settings_user_pasteurizer_template_1->set_page_header("Установка времени 2", 1);

    UI_Set1 = new UIValueSetter(
        UI_settings_user_pasteurizer_template_1,
        40, true,
        "", &img_fan
    );
    UI_Set1->set_position(0, 10, 40);
    UI_Set1->set_extra_button_logic({
        []() { ++*demo_setter_value; },
        []() { --*demo_setter_value; },
        []() { Serial.println(*demo_setter_value); }
    });
    UI_Set1->add_ui_context_action([this]() { UI_Set1->set_value(*demo_setter_value); });

    UI_Set2 = new UIValueSetter(
        UI_settings_user_pasteurizer_template_1,
        1, 40, 10, 40, false,
        "", &img_tempC
    );

    UI_Set3 = new UIValueSetter(
        UI_settings_user_pasteurizer_template_1,
        1, 40, 50, 40, false,
        "час"
    );
}
