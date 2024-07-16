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

    this->init_screens();
    this->UI_blowing_control->hide_ui_hierarchy();
    //this->UI_menu_list_user->hide_ui_hierarchy();
    this->UI_task_roadmap_control->hide_ui_hierarchy();
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
                Program_control->pause_task();
                UI_task_roadmap_control->update_ui_context();
                UI_task_roadmap_control->update_task_steps_state();
            }),
            KeyModel(KeyMap::L_STACK_1, [this]()
            {
                Program_control->end_task();
                UI_task_roadmap_control->update_ui_context();
                UI_task_roadmap_control->update_task_steps_state();
            })
        },
        this->screen
    );

    UI_task_roadmap_control->add_ui_base_action([this]() {
        if (Program_control->state != TaskStateEnum::AWAIT)
        {
            switch (Program_control->aim)
            {
            case ProgramAimEnum::TMP_PASTEUR:       UI_task_roadmap_control->set_task_header_name("Пастеризация"); break;
            case ProgramAimEnum::TMP_HEAT:          UI_task_roadmap_control->set_task_header_name("Подогрев"); break;
            case ProgramAimEnum::TMP_CHILL:         UI_task_roadmap_control->set_task_header_name("Охлаждение"); break;
            case ProgramAimEnum::TMP_WATCHDOG_1:    UI_task_roadmap_control->set_task_header_name("Будильник 1"); break;
            case ProgramAimEnum::TMP_WATCHDOG_2:    UI_task_roadmap_control->set_task_header_name("Будильник 2"); break;
            case ProgramAimEnum::TMP_WATCHDOG_3:    UI_task_roadmap_control->set_task_header_name("Будильник 3"); break;
            case ProgramAimEnum::CHM_MAIN_1:        UI_task_roadmap_control->set_task_header_name("Программа 1"); break;
            case ProgramAimEnum::CHM_MAIN_2:        UI_task_roadmap_control->set_task_header_name("Программа 2"); break;
            case ProgramAimEnum::CHM_MAIN_3:        UI_task_roadmap_control->set_task_header_name("Программа 3"); break;
            case ProgramAimEnum::CHM_TEMPL_1:       UI_task_roadmap_control->set_task_header_name("Шаблон 1"); break;
            case ProgramAimEnum::CHM_TEMPL_2:       UI_task_roadmap_control->set_task_header_name("Шаблон 2"); break;
            case ProgramAimEnum::CHM_TEMPL_3:       UI_task_roadmap_control->set_task_header_name("Шаблон 3"); break;
            case ProgramAimEnum::CHM_TEMPL_4:       UI_task_roadmap_control->set_task_header_name("Шаблон 4"); break;
            case ProgramAimEnum::CHM_TEMPL_5:       UI_task_roadmap_control->set_task_header_name("Шаблон 5"); break;
            case ProgramAimEnum::CHM_TEMPL_6:       UI_task_roadmap_control->set_task_header_name("Шаблон 6"); break;
            
            default:
                break;
            }
        }
    });

    UI_task_roadmap_control->add_ui_context_action(
        [this]()
        {
            if (Program_control->state != TaskStateEnum::AWAIT)
                UI_task_roadmap_control->set_task_state_values(
                    Program_control->get_prog_percentage(),
                    Program_control->gone_ss,
                    Program_control->state
                );
        }
    );

    if (!Program_control->is_runned)
    {
        Program_control->start_task(ProgramAimEnum::TMP_PASTEUR, &my_demo_task_steps);

        for (uint16_t i = 0; i < my_demo_task_steps.size(); i++)
        {
            ProgramStep* step = &my_demo_task_steps.at(i);

            UITaskListItem* ui_step = UI_task_roadmap_control->add_task_step(i == 0);
            ui_step->set_extra_button_logic({
                [=](){ step->fan++;             },
                [=](){ step->fan--;             },
                [=](){ step->tempC++;           },
                [=](){ step->tempC--;           },
                [=](){ step->duration_ss += 10; },
                [=](){ step->duration_ss -= 10; },
            });

            ui_step->add_ui_base_action([ui_step, step]()
            {
                switch (step->aim)
                {
                case ProgramStepAimEnum::WATER_JACKET:  ui_step->set_step_name("Набор воды"); break;
                case ProgramStepAimEnum::PASTEUR:       ui_step->set_step_name("Пастеризация"); break;
                case ProgramStepAimEnum::CHILLING:      ui_step->set_step_name("Охлаждение"); break;
                case ProgramStepAimEnum::CUTTING:       ui_step->set_step_name("Резка"); break;
                case ProgramStepAimEnum::MIXING:        ui_step->set_step_name("Замешивание"); break;
                case ProgramStepAimEnum::HEATING:       ui_step->set_step_name("Нагрев"); break;
                case ProgramStepAimEnum::DRYING:        ui_step->set_step_name("Сушка"); break;
                
                default:
                    break;
                }
            });
            ui_step->add_ui_context_action([ui_step, step]() { ui_step->set_step_values(step->fan, step->tempC, step->time_left_ss(), step->state); });
        } 

        UI_task_roadmap_control->update_ui_base();
        UI_task_roadmap_control->update_ui_context();
    }

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

    //обдумать, как загружать новые программы по примеру ниже

    if (!my_demo_task.is_runned)
    {
        my_demo_task.start_task("Пастеризация", &my_demo_task_steps);

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

        UI_task_roadmap_control->update_ui_base();
        UI_task_roadmap_control->update_ui_context();
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
                if (Blowing_control->is_runned)
                    Blowing_control->blowgun_stop();
                else
                    UI_blowing_control->navi_back();
            }),
            KeyModel(KeyMap::RIGHT_BOT_REL, [this]() { Blowing_control->blowgun_trigger(false, true); })
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
    в UIService проинициализирован demo-вектор vector<BlowgunValue> b_vars для тестов с var в RAM, пока не во FRAM
    */

    for (uint8_t i = 0; i < 4; i++)
    {
        Blow_vars.push_back(new UIBlowValListItem(UI_blowing_control, i < 3 ? BlowVarTypeEnum::LITRES : BlowVarTypeEnum::TIMER));
        UIBlowValListItem * blow_ptr = Blow_vars.back();

        blow_ptr->add_ui_context_action([=]() { blow_ptr->set_value(b_vars.at(i).val, i != 3 ? " л." : ""); });
        blow_ptr->set_extra_button_logic({
            [=]() { b_vars.at(i).val += (i != 3 ? 250 : 5); },
            [=]() { b_vars.at(i).val -= (i != 3 ? 250 : 5); },
            [=]() { Blowing_control->blowgun_trigger(true, true, i, b_vars.at(i)); }
        });
    }
}

void UIService::init_settings_user_controls()
{
    UI_settings_rt = new UIMenuListItem(UI_menu_list_user, "Дата / Время");
    UI_settings_rt->set_page_header("Установка времени", 0);

    UI_settings_pump_calibr = new UIMenuListItem(UI_menu_list_user, "Раздача жидкости");
    UI_settings_pump_calibr->set_page_header("Калибровка насоса", 0);

    UI_setter_hh = new UIValueSetter(UI_settings_rt, 0, 40, 10, 40, true, "час");
    UI_setter_hh->set_extra_button_logic({
        [this]() { *var_rt_setter.ptr() += S_Time(1, 0, 0); var_rt_setter.accept(); },
        [this]() { *var_rt_setter.ptr() -= S_Time(1, 0, 0); var_rt_setter.accept(); },
        []() {}
    });

    UI_setter_mm = new UIValueSetter(UI_settings_rt, 0, 40, 55, 40, true, "мин");
    UI_setter_mm->set_extra_button_logic({
        [this]() { *var_rt_setter.ptr() += S_Time(0, 1, 0); var_rt_setter.accept(); },
        [this]() { *var_rt_setter.ptr() -= S_Time(0, 1, 0); var_rt_setter.accept(); },
        []() {}
    });

    UI_setter_ss = new UIValueSetter(UI_settings_rt, 0, 40, 100, 40, true, "сек");
    UI_setter_ss->set_extra_button_logic({
        [this]() { *var_rt_setter.ptr() += S_Time(0, 0, 1); var_rt_setter.accept(); },
        [this]() { *var_rt_setter.ptr() -= S_Time(0, 0, 1); var_rt_setter.accept(); },
        []() {}
    });
    
    UI_setter_dd = new UIValueSetter(UI_settings_rt, 0, 40, 145, 40, true, "день");
    UI_setter_dd->set_extra_button_logic({
        [this]() { *var_rt_setter.ptr() += S_Date(1, 0, 0, true); var_rt_setter.accept(); },
        [this]() { *var_rt_setter.ptr() -= S_Date(1, 0, 0, true); var_rt_setter.accept(); },
        []() {}
    });
    
    UI_setter_MM = new UIValueSetter(UI_settings_rt, 0, 40, 190, 40, true, "мес.");
    UI_setter_MM->set_extra_button_logic({
        [this]() { *var_rt_setter.ptr() += S_Date(0, 1, 0, true); var_rt_setter.accept(); },
        [this]() { *var_rt_setter.ptr() -= S_Date(0, 1, 0, true); var_rt_setter.accept(); },
        []() {}
    });
    
    UI_setter_yyyy = new UIValueSetter(UI_settings_rt, 0, 50, 235, 40, true, "год");
    UI_setter_yyyy->set_extra_button_logic({
        [this]() { *var_rt_setter.ptr() += S_Date(0, 0, 1, true); var_rt_setter.accept(); },
        [this]() { *var_rt_setter.ptr() -= S_Date(0, 0, 1, true); var_rt_setter.accept(); },
        []() {}
    });

    UI_rt_setter_accept = new UIValueSetter(UI_settings_rt, 0, 275, 10, 140, false, "Применить", nullptr, true);
    UI_rt_setter_accept->set_extra_button_logic({
        []() {},
        []() {},
        []() {
            int year = var_rt_setter.get().get_date()->get_year() - 2000;
            
            rtc->setHour(var_rt_setter.get().get_time()->get_hours());
            rtc->setMinute(var_rt_setter.get().get_time()->get_minutes());
            rtc->setSecond(var_rt_setter.get().get_time()->get_seconds());
            rtc->setDate(var_rt_setter.get().get_date()->get_day());
            rtc->setMonth(var_rt_setter.get().get_date()->get_month());
            rtc->setYear(year < 0 ? 0 : year);
        }
    });

    UI_setter_hh->add_ui_context_action([this]() { display_rt_in_setters(); });
    UI_setter_mm->add_ui_context_action([this]() { display_rt_in_setters(); }, false);
    UI_setter_ss->add_ui_context_action([this]() { display_rt_in_setters(); }, false);
    UI_setter_dd->add_ui_context_action([this]() { display_rt_in_setters(); }, false);
    UI_setter_MM->add_ui_context_action([this]() { display_rt_in_setters(); }, false);
    UI_setter_yyyy->add_ui_context_action([this]() { display_rt_in_setters(); }, false);
}

void UIService::display_rt_in_setters()
{
    UI_setter_hh    ->set_value(var_rt_setter.get().get_time()->get_hours());
    UI_setter_mm    ->set_value(var_rt_setter.get().get_time()->get_minutes());
    UI_setter_ss    ->set_value(var_rt_setter.get().get_time()->get_seconds());
    UI_setter_dd    ->set_value(var_rt_setter.get().get_date()->get_day());
    UI_setter_MM    ->set_value(var_rt_setter.get().get_date()->get_month());
    UI_setter_yyyy  ->set_value(var_rt_setter.get().get_date()->get_year());
}