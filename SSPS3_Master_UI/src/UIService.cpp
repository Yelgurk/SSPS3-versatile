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
    this->UI_task_roadmap_control->hide_ui_hierarchy();
    this->UI_blowing_control->hide_ui_hierarchy();
    this->UI_menu_list_user->hide_ui_hierarchy();
    //this->UI_menu_list_master->hide_ui_hierarchy();
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

    /*
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

    /* master menu panel init */
    UI_menu_list_master = new UIMenuList(
        this->screen,
        {
            KeyModel(KeyMap::TOP, [this]() { UI_menu_list_master->navi_prev(); }),
            KeyModel(KeyMap::BOTTOM, [this]() { UI_menu_list_master->navi_next(); }),
            KeyModel(KeyMap::LEFT_TOP,  [this]() { UI_menu_list_master->navi_back(); }),
            KeyModel(KeyMap::LEFT_BOT,  [this]() { UI_menu_list_master->navi_ok(); })
        }
    );
    init_settings_master_controls();
}

void UIService::init_blowing_controls()
{
    /*
    b_vars - были тестовые данные для обкатки контрола раздачи. будет во FRAM 
    в UIService проинициализирован demo-вектор vector<BlowgunValue> b_vars для тестов с var в RAM, пока не во FRAM
    */

    /*
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
    */
}

void UIService::init_settings_user_controls()
{
    init_settings_part_datetime();
    init_settings_part_pump_calibration();
    init_settings_part_tmpe_templates();
    init_settings_part_tmpe_wd();
    init_settings_part_chm_templates();
}

void UIService::init_settings_part_datetime()
{
    UI_settings_rt = new UIMenuListItem(UI_menu_list_user, "Дата / Время");
    UI_settings_rt->set_page_header("Установка времени", 0);

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

    UI_rt_setter_accept = new UIValueSetter(UI_settings_rt, 0, 275, 10, 140, false, "Применить", nullptr, false, true);
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

void UIService::init_settings_part_pump_calibration()
{
    UI_settings_pump_calibr = new UIMenuListItem(UI_menu_list_user, "Раздача жидкости");
    UI_settings_pump_calibr->set_page_header("Калибровка насоса", 0);

    UI_setter_pump_calibr_lm = new UIValueSetter(UI_settings_pump_calibr, 0, 120, 90, 80, true, "доп. л/м");
    UI_setter_pump_calibr_lm->set_extra_button_logic({
        [this]() { *var_blow_pump_calibration_lm.ptr() += 0.1f; var_blow_pump_calibration_lm.accept(); },
        [this]() { *var_blow_pump_calibration_lm.ptr() -= 0.1f; var_blow_pump_calibration_lm.accept(); },
        []() {}
    });
    UI_setter_pump_calibr_lm->add_ui_context_action([this]() {
        static char str_buff[10];
        sprintf(str_buff, "%.3f л.", var_blow_pump_calibration_lm.get());
        UI_setter_pump_calibr_lm->set_value(std::string(str_buff));
    });
}

void UIService::init_settings_part_tmpe_templates()
{
    menu_tmpe_general_start_at = UI_menu_list_user->get_childs_count();
    menu_tmpe_local_start_at = UI_template_menu_items.size();

    static vector<std::string> tmpe_templ_menu_items_name = {
        "программа \"Пастеризация\"",
        "программа \"Охлаждение\"",
        "программа \"Нагрев\""
    };

    static vector<std::string> tmpe_templ_step_name = {
        "1. пастер.",
        "2. охлажд",
        "3. нагрев."
    };

    for (uint8_t i = 0; i < TEMPLATES_COUNT_TMPE; i++)
    {
        UI_template_menu_items.push_back(new UIMenuListItem(
            UI_menu_list_user,
            i < tmpe_templ_menu_items_name.size() ?
            tmpe_templ_menu_items_name.at(i) :
            "Авто прогр. #" + to_string(i - tmpe_templ_menu_items_name.size() + 1),
            i == 0
        ));
    }

    UIValueSetter * setter_turn_on_off;
    UIValueSetter * setter_fan_speed;
    UIValueSetter * setter_tempC;
    UIValueSetter * setter_durat_ss;
    UIValueSetter * setter_pause_after;

    UI_template_menu_items.at(menu_tmpe_local_start_at)->set_page_header("Поэтапная настройка", 0);

    for (uint8_t page = 0; page < 3; page++)
    {
        UI_template_setters.push_back(new UIValueSetter(UI_template_menu_items.at(menu_tmpe_local_start_at), 0, 80, 10, 40 + (100 * page), page == 0, tmpe_templ_step_name.at(page), nullptr, false));
        setter_turn_on_off = UI_template_setters.back();

        UI_template_setters.push_back(new UIValueSetter(UI_template_menu_items.at(menu_tmpe_local_start_at), 0, 40, 95, 40 + (100 * page), page == 0, "", &img_fan));
        setter_fan_speed = UI_template_setters.back();

        UI_template_setters.push_back(new UIValueSetter(UI_template_menu_items.at(menu_tmpe_local_start_at), 0, 40, 140, 40 + (100 * page), page == 0, "", &img_tempC));
        setter_tempC = UI_template_setters.back();

        UI_template_setters.push_back(new UIValueSetter(UI_template_menu_items.at(menu_tmpe_local_start_at), 0, 40, 185, 40 + (100 * page), page == 0, "", &img_sand_watch));
        setter_durat_ss = UI_template_setters.back();

        UI_template_setters.push_back(new UIValueSetter(UI_template_menu_items.at(menu_tmpe_local_start_at), 0, 55, 230, 40 + (100 * page), page == 0, "пауза", nullptr, false));
        setter_pause_after = UI_template_setters.back();

        /* ON/OFF step processing setter */
        setter_turn_on_off->set_extra_button_logic({
            []() {},
            []() {},
            [this, page]() {
                TMPEProgramTemplate * templ = prog_tmpe_templates->at(get_template_tmpe_index())->ptr();
                templ->get_step(page)->step_is_turned_on = !templ->get_step(page)->step_is_turned_on;
                prog_tmpe_templates->at(get_template_tmpe_index())->accept();
            }
        });
        setter_turn_on_off->add_ui_context_action([=]() {
            bool state = prog_tmpe_templates->at(get_template_tmpe_index())->get().get_step(page)->step_is_turned_on;
            setter_turn_on_off->set_value(state ? "ВКЛ" : "ВЫКЛ");
        });

        /* Async motor RPM setter */
        setter_fan_speed->set_extra_button_logic({
            [this, page]() {
                TMPEProgramTemplate * templ = prog_tmpe_templates->at(get_template_tmpe_index())->ptr();
                templ->get_step(page)->fan++;
                prog_tmpe_templates->at(get_template_tmpe_index())->accept();
            },
            [this, page]() {
                TMPEProgramTemplate * templ = prog_tmpe_templates->at(get_template_tmpe_index())->ptr();
                templ->get_step(page)->fan--;
                prog_tmpe_templates->at(get_template_tmpe_index())->accept();
            },
            []() {}
        });
        setter_fan_speed->add_ui_context_action([=]() {
            setter_fan_speed->set_value(prog_tmpe_templates->at(get_template_tmpe_index())->get().get_step(page)->fan);
        });

        /* Step temperature limit */
        setter_tempC->set_extra_button_logic({
            [this, page]() {
                TMPEProgramTemplate * templ = prog_tmpe_templates->at(get_template_tmpe_index())->ptr();
                templ->get_step(page)->tempC++;
                prog_tmpe_templates->at(get_template_tmpe_index())->accept();
            },
            [this, page]() {
                TMPEProgramTemplate * templ = prog_tmpe_templates->at(get_template_tmpe_index())->ptr();
                templ->get_step(page)->tempC--;
                prog_tmpe_templates->at(get_template_tmpe_index())->accept();
            },
            []() {}
        });
        setter_tempC->add_ui_context_action([=]() {
            setter_tempC->set_value(prog_tmpe_templates->at(get_template_tmpe_index())->get().get_step(page)->tempC);
        });

        /* Step duration SS setter */
        setter_durat_ss->set_extra_button_logic({
            [this, page]() {
                TMPEProgramTemplate * templ = prog_tmpe_templates->at(get_template_tmpe_index())->ptr();
                templ->get_step(page)->duration_ss++;
                prog_tmpe_templates->at(get_template_tmpe_index())->accept();
            },
            [this, page]() {
                TMPEProgramTemplate * templ = prog_tmpe_templates->at(get_template_tmpe_index())->ptr();
                templ->get_step(page)->duration_ss--;
                prog_tmpe_templates->at(get_template_tmpe_index())->accept();
            },
            []() {}
        });
        setter_durat_ss->add_ui_context_action([=]() {
            setter_durat_ss->set_value((int32_t)prog_tmpe_templates->at(get_template_tmpe_index())->get().get_step(page)->duration_ss);
        });

        /* ON/OFF pause after step completed setter */
        setter_pause_after->set_extra_button_logic({
            []() {},
            []() {},
            [this, page]() {
                TMPEProgramTemplate * templ = prog_tmpe_templates->at(get_template_tmpe_index())->ptr();
                templ->get_step(page)->await_ok_button = !templ->get_step(page)->await_ok_button;
                prog_tmpe_templates->at(get_template_tmpe_index())->accept();
            }
        });
        setter_pause_after->add_ui_context_action([=]() {
            bool state = prog_tmpe_templates->at(get_template_tmpe_index())->get().get_step(page)->await_ok_button;
            setter_pause_after->set_value(state ? "ВКЛ" : "ВЫКЛ");
        });
    } 

    for (uint8_t i = menu_tmpe_local_start_at + 1; i < UI_template_menu_items.size(); i++)
        UI_template_menu_items.at(i)->set_childs_presenter(UI_template_menu_items.at(menu_tmpe_local_start_at));
}

void UIService::init_settings_part_tmpe_wd()
{
    menu_wd_general_start_at = UI_menu_list_user->get_childs_count();
    menu_wd_local_start_at = UI_template_menu_items.size();

    for (uint8_t i = 0; i < TEMPLATES_COUNT_WD; i++)
        UI_template_menu_items.push_back(new UIMenuListItem(UI_menu_list_user, "Авто ВКЛ прогр. #" + to_string(i + 1), i == 0));
    UI_template_menu_items.at(menu_wd_local_start_at)->set_page_header("Настр. включения по времени", 0);

    UI_template_setters.push_back(new UIValueSetter(UI_template_menu_items.at(menu_wd_local_start_at), 0, 80, 10, 40, true, "Запущено", nullptr, false));
    UI_setter_wd_turn_on_off = UI_template_setters.back();
    
    UI_template_setters.push_back(new UIValueSetter(UI_template_menu_items.at(menu_wd_local_start_at), 0, 40, 95, 40, true, "час"));
    UI_setter_wd_hh = UI_template_setters.back();
    
    UI_template_setters.push_back(new UIValueSetter(UI_template_menu_items.at(menu_wd_local_start_at), 0, 40, 140, 40, true, "мин."));
    UI_setter_wd_mm = UI_template_setters.back();
    
    UI_template_setters.push_back(new UIValueSetter(UI_template_menu_items.at(menu_wd_local_start_at), 0, 40, 185, 40, true, "сек"));
    UI_setter_wd_ss = UI_template_setters.back();
    
    /* ON/OFF template watchdog (auto startup by time) */
    UI_setter_wd_turn_on_off->set_extra_button_logic({
        []() {},
        []() {},
        [this]() {
            bool * templ = prog_tmpe_templates_wd_state->at(get_template_wd_index())->ptr();
            *templ = !*templ;
            prog_tmpe_templates_wd_state->at(get_template_wd_index())->accept();
        }
    });
    UI_setter_wd_turn_on_off->add_ui_context_action([=]() {
        UI_setter_wd_turn_on_off->set_value(prog_tmpe_templates_wd_state->at(get_template_wd_index())->get() ? "Да" : "Нет");
    });

    /* WD startup hours */
    UI_setter_wd_hh->set_extra_button_logic({
        [this]() {
            S_Time * wd_hh = prog_tmpe_templates_wd_time->at(get_template_wd_index())->ptr();
            *wd_hh += S_Time(1, 0, 0);
            prog_tmpe_templates_wd_time->at(get_template_wd_index())->accept();
        },
        [this]() {
            
            S_Time * wd_hh = prog_tmpe_templates_wd_time->at(get_template_wd_index())->ptr();
            *wd_hh -= S_Time(1, 0, 0);
            prog_tmpe_templates_wd_time->at(get_template_wd_index())->accept();
        },
        []() {}
    });
    UI_setter_wd_hh->add_ui_context_action([this]() { display_wd_in_setters(); });
    
    /* WD startup minutes */
    UI_setter_wd_mm->set_extra_button_logic({
        [this]() {
            S_Time * wd_hh = prog_tmpe_templates_wd_time->at(get_template_wd_index())->ptr();
            *wd_hh += S_Time(0, 1, 0);
            prog_tmpe_templates_wd_time->at(get_template_wd_index())->accept();
        },
        [this]() {
            
            S_Time * wd_hh = prog_tmpe_templates_wd_time->at(get_template_wd_index())->ptr();
            *wd_hh -= S_Time(0, 1, 0);
            prog_tmpe_templates_wd_time->at(get_template_wd_index())->accept();
        },
        []() {}
    });
    UI_setter_wd_mm->add_ui_context_action([this]() { display_wd_in_setters(); });

    /* WD startup seconds */
    UI_setter_wd_ss->set_extra_button_logic({
        [this]() {
            S_Time * wd_hh = prog_tmpe_templates_wd_time->at(get_template_wd_index())->ptr();
            *wd_hh += S_Time(0, 0, 1);
            prog_tmpe_templates_wd_time->at(get_template_wd_index())->accept();
        },
        [this]() {
            
            S_Time * wd_hh = prog_tmpe_templates_wd_time->at(get_template_wd_index())->ptr();
            *wd_hh -= S_Time(0, 0, 1);
            prog_tmpe_templates_wd_time->at(get_template_wd_index())->accept();
        },
        []() {}
    });
    UI_setter_wd_ss->add_ui_context_action([this]() { display_wd_in_setters(); });
    
    for (uint8_t i = menu_wd_local_start_at + 1; i < UI_template_menu_items.size(); i++)
        UI_template_menu_items.at(i)->set_childs_presenter(UI_template_menu_items.at(menu_wd_local_start_at));
}

void UIService::init_settings_part_chm_templates()
{
    menu_chm_general_start_at = UI_menu_list_user->get_childs_count();
    menu_chm_local_start_at = UI_template_menu_items.size();

    static vector<std::string> chm_templ_menu_items_name = {
        "сыр \"Пармезан\"",
        "сыр \"Тильзитский\"",
        "сыр \"Адыгейский\""
    };

    static vector<std::string> chm_templ_step_name = {
        "1. пастер.",
        "2. охлажд",
        "3. резка",
        "4. замеш.",
        "5. нагрев",
        "6. сушка"
    };

    for (uint8_t i = 0; i < TEMPLATES_COUNT_CHM; i++)
    {
        UI_template_menu_items.push_back(new UIMenuListItem(
            UI_menu_list_user,
            i < chm_templ_menu_items_name.size() ?
            chm_templ_menu_items_name.at(i) :
            "свой рецепт #" + to_string(i - chm_templ_menu_items_name.size() + 1),
            i == 0
        ));
    }

    UIValueSetter * setter_turn_on_off;
    UIValueSetter * setter_fan_speed;
    UIValueSetter * setter_tempC;
    UIValueSetter * setter_durat_ss;
    UIValueSetter * setter_pause_after;

    UI_template_menu_items.at(menu_chm_local_start_at)->set_page_header("Поэтапная настройка", 0);

    for (uint8_t page = 0; page < 6; page++)
    {
        UI_template_setters.push_back(new UIValueSetter(UI_template_menu_items.at(menu_chm_local_start_at), 0, 80, 10, 40 + (100 * page), page == 0, chm_templ_step_name.at(page), nullptr, false));
        setter_turn_on_off = UI_template_setters.back();

        UI_template_setters.push_back(new UIValueSetter(UI_template_menu_items.at(menu_chm_local_start_at), 0, 40, 95, 40 + (100 * page), page == 0, "", &img_fan));
        setter_fan_speed = UI_template_setters.back();

        UI_template_setters.push_back(new UIValueSetter(UI_template_menu_items.at(menu_chm_local_start_at), 0, 40, 140, 40 + (100 * page), page == 0, "", &img_tempC));
        setter_tempC = UI_template_setters.back();

        UI_template_setters.push_back(new UIValueSetter(UI_template_menu_items.at(menu_chm_local_start_at), 0, 40, 185, 40 + (100 * page), page == 0, "", &img_sand_watch));
        setter_durat_ss = UI_template_setters.back();

        UI_template_setters.push_back(new UIValueSetter(UI_template_menu_items.at(menu_chm_local_start_at), 0, 55, 230, 40 + (100 * page), page == 0, "пауза", nullptr, false));
        setter_pause_after = UI_template_setters.back();

        /* ON/OFF step processing setter */
        setter_turn_on_off->set_extra_button_logic({
            []() {},
            []() {},
            [this, page]() {
                CHMProgramTemplate * templ = prog_chm_templates->at(get_template_chm_index())->ptr();
                templ->get_step(page)->step_is_turned_on = !templ->get_step(page)->step_is_turned_on;
                prog_chm_templates->at(get_template_chm_index())->accept();
            }
        });
        setter_turn_on_off->add_ui_context_action([=]() {
            bool state = prog_chm_templates->at(get_template_chm_index())->get().get_step(page)->step_is_turned_on;
            setter_turn_on_off->set_value(state ? "ВКЛ" : "ВЫКЛ");
        });

        /* Async motor RPM setter */
        setter_fan_speed->set_extra_button_logic({
            [this, page]() {
                CHMProgramTemplate * templ = prog_chm_templates->at(get_template_chm_index())->ptr();
                templ->get_step(page)->fan++;
                prog_chm_templates->at(get_template_chm_index())->accept();
            },
            [this, page]() {
                CHMProgramTemplate * templ = prog_chm_templates->at(get_template_chm_index())->ptr();
                templ->get_step(page)->fan--;
                prog_chm_templates->at(get_template_chm_index())->accept();
            },
            []() {}
        });
        setter_fan_speed->add_ui_context_action([=]() {
            setter_fan_speed->set_value(prog_chm_templates->at(get_template_chm_index())->get().get_step(page)->fan);
        });

        /* Step temperature limit */
        setter_tempC->set_extra_button_logic({
            [this, page]() {
                CHMProgramTemplate * templ = prog_chm_templates->at(get_template_chm_index())->ptr();
                templ->get_step(page)->tempC++;
                prog_chm_templates->at(get_template_chm_index())->accept();
            },
            [this, page]() {
                CHMProgramTemplate * templ = prog_chm_templates->at(get_template_chm_index())->ptr();
                templ->get_step(page)->tempC--;
                prog_chm_templates->at(get_template_chm_index())->accept();
            },
            []() {}
        });
        setter_tempC->add_ui_context_action([=]() {
            setter_tempC->set_value(prog_chm_templates->at(get_template_chm_index())->get().get_step(page)->tempC);
        });

        /* Step duration SS setter */
        setter_durat_ss->set_extra_button_logic({
            [this, page]() {
                CHMProgramTemplate * templ = prog_chm_templates->at(get_template_chm_index())->ptr();
                templ->get_step(page)->duration_ss++;
                prog_chm_templates->at(get_template_chm_index())->accept();
            },
            [this, page]() {
                CHMProgramTemplate * templ = prog_chm_templates->at(get_template_chm_index())->ptr();
                templ->get_step(page)->duration_ss--;
                prog_chm_templates->at(get_template_chm_index())->accept();
            },
            []() {}
        });
        setter_durat_ss->add_ui_context_action([=]() {
            setter_durat_ss->set_value((int32_t)prog_chm_templates->at(get_template_chm_index())->get().get_step(page)->duration_ss);
        });

        /* ON/OFF pause after step completed setter */
        setter_pause_after->set_extra_button_logic({
            []() {},
            []() {},
            [this, page]() {
                CHMProgramTemplate * templ = prog_chm_templates->at(get_template_chm_index())->ptr();
                templ->get_step(page)->await_ok_button = !templ->get_step(page)->await_ok_button;
                prog_chm_templates->at(get_template_chm_index())->accept();
            }
        });
        setter_pause_after->add_ui_context_action([=]() {
            bool state = prog_chm_templates->at(get_template_chm_index())->get().get_step(page)->await_ok_button;
            setter_pause_after->set_value(state ? "ВКЛ" : "ВЫКЛ");
        });
    } 

    for (uint8_t i = menu_chm_local_start_at + 1; i < UI_template_menu_items.size(); i++)
        UI_template_menu_items.at(i)->set_childs_presenter(UI_template_menu_items.at(menu_chm_local_start_at));
}

void UIService::init_settings_master_controls()
{
    UI_settings_master_machine          = new UIMenuListItem(UI_menu_list_master, "Оборудование");
    UI_settings_master_sensors          = new UIMenuListItem(UI_menu_list_master, "Сенсоры");
    UI_settings_master_limits_blowing   = new UIMenuListItem(UI_menu_list_master, "Насос");
    UI_settings_master_limits_prog      = new UIMenuListItem(UI_menu_list_master, "Программы");
    UI_settings_master_machine          ->set_page_header("Модель, язык и \"железо\"", 0);
    UI_settings_master_machine          ->set_page_header("Опасный раздел", 1);
    UI_settings_master_sensors          ->set_page_header("Надстройка датчиков", 0);
    UI_settings_master_limits_blowing   ->set_page_header("Надстройка насоса", 0);
    UI_settings_master_limits_prog      ->set_page_header("Надстройка программ\n#АП - \"Авария после\"", 0);

    /* master, page - 1 */
    UI_S_M_type_of_equipment_enum = new UIValueSetter(UI_settings_master_machine, true, 40, "модель");
    UI_S_M_type_of_equipment_enum->set_extra_button_logic({
        [this]() { var_type_of_equipment_enum.set(var_type_of_equipment_enum.get() + 1); },
        [this]() { var_type_of_equipment_enum.set(var_type_of_equipment_enum.get() - 1); },
        []() {}
    });
    UI_S_M_type_of_equipment_enum->add_ui_context_action([this]() { UI_S_M_type_of_equipment_enum->set_value(var_type_of_equipment_enum.get()); });

    UI_S_M_is_blowgun_by_rf = new UIValueSetter(UI_settings_master_machine, false, 40, "пистолет");
    UI_S_M_is_blowgun_by_rf->set_extra_button_logic({
        [this]() { var_is_blowgun_by_rf.set(var_is_blowgun_by_rf.get() - 1); },
        [this]() { var_is_blowgun_by_rf.set(var_is_blowgun_by_rf.get() - 1); },
        []() {}
    });
    UI_S_M_is_blowgun_by_rf->add_ui_context_action([this]() { UI_S_M_is_blowgun_by_rf->set_value(
        var_is_blowgun_by_rf.get()
        ? "радио"
        : "кнопка"    
    ); });

    UI_S_M_is_asyncM_rpm_float = new UIValueSetter(UI_settings_master_machine, false, 40, "частотник");
    UI_S_M_is_asyncM_rpm_float->set_extra_button_logic({
        [this]() { var_is_asyncM_rpm_float.set(var_is_asyncM_rpm_float.get() - 1); },
        [this]() { var_is_asyncM_rpm_float.set(var_is_asyncM_rpm_float.get() - 1); },
        []() {}
    });
    UI_S_M_is_asyncM_rpm_float->add_ui_context_action([this]() { UI_S_M_is_asyncM_rpm_float->set_value(
        var_is_asyncM_rpm_float.get()
        ? "есть"
        : "нет"
    ); });

    UI_S_M_plc_language = new UIValueSetter(UI_settings_master_machine, true, 40, "язык системы");
    UI_S_M_plc_language->set_extra_button_logic({
        [this]() { var_plc_language.set(var_plc_language.get() + 1); },
        [this]() { var_plc_language.set(var_plc_language.get() - 1); },
        []() {}
    });
    UI_S_M_plc_language->add_ui_context_action([this]() { UI_S_M_plc_language->set_value(var_plc_language.get()); });

    UI_S_M_reboot_system = new UIValueSetter(UI_settings_master_machine, 1, 265, 10, 40, false, "Перезапуск системы", nullptr, false, true);
    UI_S_M_reboot_system->set_extra_button_logic({
        []() {},
        []() {},
        []() { ESP.restart(); }
    });

    UI_S_M_reset_system = new UIValueSetter(UI_settings_master_machine, 1, 265, 10, 95, false, "Полный сброс + перезапуск", nullptr, false, true);
    UI_S_M_reset_system->set_extra_button_logic({
        []() {},
        []() {},
        []() {
            Storage::reset_all();
            ESP.restart();
        }
    });
    
    /* master, page - 2 */   
    UI_S_M_sensor_batt_min_V = new UIValueSetter(UI_settings_master_sensors, true, 40, "Аккум.\nmin V", nullptr, &OpenSans_bold_14px);
    UI_S_M_sensor_batt_min_V->set_extra_button_logic({
        [this]() { var_sensor_batt_min_V.set(var_sensor_batt_min_V.get() + 0.1f); },
        [this]() { var_sensor_batt_min_V.set(var_sensor_batt_min_V.get() - 0.1f); },
        []() {}
    });
    UI_S_M_sensor_batt_min_V->add_ui_context_action([this]() { UI_S_M_sensor_batt_min_V->set_value(var_sensor_batt_min_V.get(), "v", 1); });

    UI_S_M_sensor_batt_max_V = new UIValueSetter(UI_settings_master_sensors, true, 40, "Аккум.\nmax V", nullptr, &OpenSans_bold_14px);
    UI_S_M_sensor_batt_max_V->set_extra_button_logic({
        [this]() { var_sensor_batt_max_V.set(var_sensor_batt_max_V.get() + 0.1f); },
        [this]() { var_sensor_batt_max_V.set(var_sensor_batt_max_V.get() - 0.1f); },
        []() {}
    });
    UI_S_M_sensor_batt_max_V->add_ui_context_action([this]() { UI_S_M_sensor_batt_max_V->set_value(var_sensor_batt_max_V.get(), "v", 1); });

    UI_S_M_sensor_batt_V_min_12bit = new UIValueSetter(UI_settings_master_sensors, true, 40, "Аккум.\nmin", nullptr, &OpenSans_bold_14px);
    UI_S_M_sensor_batt_V_min_12bit->set_extra_button_logic({
        [this]() { var_sensor_batt_V_min_12bit.set(var_sensor_batt_V_min_12bit.get() + 1); },
        [this]() { var_sensor_batt_V_min_12bit.set(var_sensor_batt_V_min_12bit.get() - 1); },
        []() {}
    });
    UI_S_M_sensor_batt_V_min_12bit->add_ui_context_action([this]() { UI_S_M_sensor_batt_V_min_12bit->set_value(var_sensor_batt_V_min_12bit.get()); });

    UI_S_M_sensor_batt_V_max_12bit = new UIValueSetter(UI_settings_master_sensors, true, 40, "Аккум.\nmax", nullptr, &OpenSans_bold_14px);
    UI_S_M_sensor_batt_V_max_12bit->set_extra_button_logic({
        [this]() { var_sensor_batt_V_max_12bit.set(var_sensor_batt_V_max_12bit.get() + 1); },
        [this]() { var_sensor_batt_V_max_12bit.set(var_sensor_batt_V_max_12bit.get() - 1); },
        []() {}
    });
    UI_S_M_sensor_batt_V_max_12bit->add_ui_context_action([this]() { UI_S_M_sensor_batt_V_max_12bit->set_value(var_sensor_batt_V_max_12bit.get()); });

    UI_S_M_sensor_tempC_limit_4ma_12bit = new UIValueSetter(UI_settings_master_sensors, true, 40, "PT100\n4ma", nullptr, &OpenSans_bold_14px);
    UI_S_M_sensor_tempC_limit_4ma_12bit->set_extra_button_logic({
        [this]() { var_sensor_tempC_limit_4ma_12bit.set(var_sensor_tempC_limit_4ma_12bit.get() + 1); },
        [this]() { var_sensor_tempC_limit_4ma_12bit.set(var_sensor_tempC_limit_4ma_12bit.get() - 1); },
        []() {}
    });
    UI_S_M_sensor_tempC_limit_4ma_12bit->add_ui_context_action([this]() { UI_S_M_sensor_tempC_limit_4ma_12bit->set_value(var_sensor_tempC_limit_4ma_12bit.get()); });

    UI_S_M_sensor_tempC_limit_20ma_12bit = new UIValueSetter(UI_settings_master_sensors, true, 40, "PT100\n20ma", nullptr, &OpenSans_bold_14px);
    UI_S_M_sensor_tempC_limit_20ma_12bit->set_extra_button_logic({
        [this]() { var_sensor_tempC_limit_20ma_12bit.set(var_sensor_tempC_limit_20ma_12bit.get() + 1); },
        [this]() { var_sensor_tempC_limit_20ma_12bit.set(var_sensor_tempC_limit_20ma_12bit.get() - 1); },
        []() {}
    });
    UI_S_M_sensor_tempC_limit_20ma_12bit->add_ui_context_action([this]() { UI_S_M_sensor_tempC_limit_20ma_12bit->set_value(var_sensor_tempC_limit_20ma_12bit.get()); });

    UI_S_M_sensor_tempC_limit_4ma_degrees_C = new UIValueSetter(UI_settings_master_sensors, true, 40, "PT100\n4ma°C", nullptr, &OpenSans_bold_14px);
    UI_S_M_sensor_tempC_limit_4ma_degrees_C->set_extra_button_logic({
        [this]() { var_sensor_tempC_limit_4ma_degrees_C.set(var_sensor_tempC_limit_4ma_degrees_C.get() + 1); },
        [this]() { var_sensor_tempC_limit_4ma_degrees_C.set(var_sensor_tempC_limit_4ma_degrees_C.get() - 1); },
        []() {}
    });
    UI_S_M_sensor_tempC_limit_4ma_degrees_C->add_ui_context_action([this]() {
        UI_S_M_sensor_tempC_limit_4ma_degrees_C->set_value(var_sensor_tempC_limit_4ma_degrees_C.get(), "°");
    });

    UI_S_M_sensor_tempC_limit_20ma_degrees_C = new UIValueSetter(UI_settings_master_sensors, true, 40, "PT100\n20ma°C", nullptr, &OpenSans_bold_14px);
    UI_S_M_sensor_tempC_limit_20ma_degrees_C->set_extra_button_logic({
        [this]() { var_sensor_tempC_limit_20ma_degrees_C.set(var_sensor_tempC_limit_20ma_degrees_C.get() + 1); },
        [this]() { var_sensor_tempC_limit_20ma_degrees_C.set(var_sensor_tempC_limit_20ma_degrees_C.get() - 1); },
        []() {}
    });
    UI_S_M_sensor_tempC_limit_20ma_degrees_C->add_ui_context_action([this]() {
        UI_S_M_sensor_tempC_limit_20ma_degrees_C->set_value(var_sensor_tempC_limit_20ma_degrees_C.get(), "°");
    });

    UI_S_M_sensor_dac_rpm_limit_min_12bit = new UIValueSetter(UI_settings_master_sensors, true, 40, "Мотор\nmin цап", nullptr, &OpenSans_bold_14px);
    UI_S_M_sensor_dac_rpm_limit_min_12bit->set_extra_button_logic({
        [this]() { uint16_t val = var_sensor_dac_rpm_limit_min_12bit.get() + 1; var_sensor_dac_rpm_limit_min_12bit.set(val > 4095 ? 0 : val); },
        [this]() { uint16_t val = var_sensor_dac_rpm_limit_min_12bit.get() - 1; var_sensor_dac_rpm_limit_min_12bit.set(val > 4095 ? 4095 : val); },
        []() {}
    });
    UI_S_M_sensor_dac_rpm_limit_min_12bit->add_ui_context_action([this]() { UI_S_M_sensor_dac_rpm_limit_min_12bit->set_value(var_sensor_dac_rpm_limit_min_12bit.get()); });

    UI_S_M_sensor_dac_rpm_limit_max_12bit = new UIValueSetter(UI_settings_master_sensors, true, 40, "Мотор\nmax цап", nullptr, &OpenSans_bold_14px);
    UI_S_M_sensor_dac_rpm_limit_max_12bit->set_extra_button_logic({
        [this]() { uint16_t val = var_sensor_dac_rpm_limit_max_12bit.get() + 1; var_sensor_dac_rpm_limit_max_12bit.set(val > 4095 ? 0 : val); },
        [this]() { uint16_t val = var_sensor_dac_rpm_limit_max_12bit.get() - 1; var_sensor_dac_rpm_limit_max_12bit.set(val > 4095 ? 4095 : val); },
        []() {}
    });
    UI_S_M_sensor_dac_rpm_limit_max_12bit->add_ui_context_action([this]() { UI_S_M_sensor_dac_rpm_limit_max_12bit->set_value(var_sensor_dac_rpm_limit_max_12bit.get()); });

    UI_S_M_sensor_dac_asyncM_rpm_min = new UIValueSetter(UI_settings_master_sensors, true, 40, "Мотор\nmin об.", nullptr, &OpenSans_bold_14px);
    UI_S_M_sensor_dac_asyncM_rpm_min->set_extra_button_logic({
        [this]() { var_sensor_dac_asyncM_rpm_min.set(var_sensor_dac_asyncM_rpm_min.get() + 1); },
        [this]() { var_sensor_dac_asyncM_rpm_min.set(var_sensor_dac_asyncM_rpm_min.get() - 1); },
        []() {}
    });
    UI_S_M_sensor_dac_asyncM_rpm_min->add_ui_context_action([this]() { UI_S_M_sensor_dac_asyncM_rpm_min->set_value(var_sensor_dac_asyncM_rpm_min.get()); });

    UI_S_M_sensor_dac_asyncM_rpm_max = new UIValueSetter(UI_settings_master_sensors, true, 40, "Мотор\nmax об.", nullptr, &OpenSans_bold_14px);
    UI_S_M_sensor_dac_asyncM_rpm_max->set_extra_button_logic({
        [this]() { var_sensor_dac_asyncM_rpm_max.set(var_sensor_dac_asyncM_rpm_max.get() + 1); },
        [this]() { var_sensor_dac_asyncM_rpm_max.set(var_sensor_dac_asyncM_rpm_max.get() - 1); },
        []() {}
    });
    UI_S_M_sensor_dac_asyncM_rpm_max->add_ui_context_action([this]() { UI_S_M_sensor_dac_asyncM_rpm_max->set_value(var_sensor_dac_asyncM_rpm_max.get()); });


    /* master, page - 3 */   
    UI_S_M_blowing_await_ss = new UIValueSetter(UI_settings_master_limits_blowing, true, 40, "Простой");
    UI_S_M_blowing_await_ss->set_extra_button_logic({
        [this]() { var_blowing_await_ss.set(var_blowing_await_ss.get() + 1); },
        [this]() { var_blowing_await_ss.set(var_blowing_await_ss.get() - 1); },
        []() {}
    });
    UI_S_M_blowing_await_ss->add_ui_context_action([this]() { UI_S_M_blowing_await_ss->set_value(var_blowing_await_ss.get(), " сек."); });

    UI_S_M_blowing_pump_power_lm = new UIValueSetter(UI_settings_master_limits_blowing, true, 40, "Объём л/м");
    UI_S_M_blowing_pump_power_lm->set_extra_button_logic({
        [this]() { float val = var_blowing_pump_power_lm.get() + 0.25f; var_blowing_pump_power_lm.set(val < 1.0f ? 1.0f : val); },
        [this]() { float val = var_blowing_pump_power_lm.get() - 0.25f; var_blowing_pump_power_lm.set(val < 1.0f ? 1.0f : val); },
        []() {}
    });
    UI_S_M_blowing_pump_power_lm->add_ui_context_action([this]() { UI_S_M_blowing_pump_power_lm->set_value(var_blowing_pump_power_lm.get(), " л.", 3); });

    UI_S_M_blowing_limit_ml_max = new UIValueSetter(UI_settings_master_limits_blowing, true, 40, "Порция max");
    UI_S_M_blowing_limit_ml_max->set_extra_button_logic({
        [this]() { var_blowing_limit_ml_max.set(var_blowing_limit_ml_max.get() + 250); },
        [this]() { var_blowing_limit_ml_max.set(var_blowing_limit_ml_max.get() - 250); },
        []() {}
    });
    UI_S_M_blowing_limit_ml_max->add_ui_context_action([this]() { UI_S_M_blowing_limit_ml_max->set_value(var_blowing_limit_ml_max.get()/1000.f, " л.", 3); });

    UI_S_M_blowing_limit_ml_min = new UIValueSetter(UI_settings_master_limits_blowing, true, 40, "Порция min");
    UI_S_M_blowing_limit_ml_min->set_extra_button_logic({
        [this]() { var_blowing_limit_ml_min.set(var_blowing_limit_ml_min.get() + 250); },
        [this]() { var_blowing_limit_ml_min.set(var_blowing_limit_ml_min.get() - 250); },
        []() {}
    });
    UI_S_M_blowing_limit_ml_min->add_ui_context_action([this]() { UI_S_M_blowing_limit_ml_min->set_value(var_blowing_limit_ml_min.get()/1000.f, " л.", 3); });

    UI_S_M_blowing_limit_ss_max = new UIValueSetter(UI_settings_master_limits_blowing, true, 40, "Мойка max");
    UI_S_M_blowing_limit_ss_max->set_extra_button_logic({
        [this]() { var_blowing_limit_ss_max.set(var_blowing_limit_ss_max.get() + 5); },
        [this]() { var_blowing_limit_ss_max.set(var_blowing_limit_ss_max.get() - 5); },
        []() {}
    });
    UI_S_M_blowing_limit_ss_max->add_ui_context_action([this]() {
        static char buffer[20];
        sprintf(buffer, "%01dм. %02dс.", var_blowing_limit_ss_max.get() / 60, var_blowing_limit_ss_max.get() % 60);\
        UI_S_M_blowing_limit_ss_max->set_value(std::string(buffer));
    });

    UI_S_M_blowing_limit_ss_min = new UIValueSetter(UI_settings_master_limits_blowing, true, 40, "Мойка min");
    UI_S_M_blowing_limit_ss_min->set_extra_button_logic({
        [this]() { var_blowing_limit_ss_min.set(var_blowing_limit_ss_min.get() + 5); },
        [this]() { var_blowing_limit_ss_min.set(var_blowing_limit_ss_min.get() - 5); },
        []() {}
    });
    UI_S_M_blowing_limit_ss_min->add_ui_context_action([this]() {
        static char buffer[20];
        sprintf(buffer, "%01dм. %02dс.", var_blowing_limit_ss_min.get() / 60, var_blowing_limit_ss_min.get() % 60);\
        UI_S_M_blowing_limit_ss_min->set_value(std::string(buffer));    
    });

    
    /* master, page - 4 */    
    UI_S_M_wJacket_tempC_limit_max = new UIValueSetter(UI_settings_master_limits_prog, true, 40, "Рубашка\nmax°C воды", nullptr, &OpenSans_bold_14px);
    UI_S_M_wJacket_tempC_limit_max->set_extra_button_logic({
        [this]() { uint8_t var = var_wJacket_tempC_limit_max.get() + 1; var_wJacket_tempC_limit_max.set(var > LIMIT_WATER_BOILING_POINT_TEMPC ? LIMIT_WATER_BOILING_POINT_TEMPC : var); },
        [this]() { uint8_t var = var_wJacket_tempC_limit_max.get() - 1; var_wJacket_tempC_limit_max.set(var > LIMIT_WATER_BOILING_POINT_TEMPC ? LIMIT_WATER_BOILING_POINT_TEMPC : var); },
        []() {}
    });
    UI_S_M_wJacket_tempC_limit_max->add_ui_context_action([this]() { UI_S_M_wJacket_tempC_limit_max->set_value(var_wJacket_tempC_limit_max.get(), "°C"); });

    UI_S_M_prog_wJacket_drain_max_ss = new UIValueSetter(UI_settings_master_limits_prog, true, 40, "Рубашка\nпусто? #АП", nullptr, &OpenSans_bold_14px);
    UI_S_M_prog_wJacket_drain_max_ss->set_extra_button_logic({
        [this]() { var_prog_wJacket_drain_max_ss.set(var_prog_wJacket_drain_max_ss.get() + 1); },
        [this]() { var_prog_wJacket_drain_max_ss.set(var_prog_wJacket_drain_max_ss.get() - 1); },
        []() {}
    });
    UI_S_M_prog_wJacket_drain_max_ss->add_ui_context_action([this]() { UI_S_M_prog_wJacket_drain_max_ss->set_value(var_prog_wJacket_drain_max_ss.get(), "сек."); });

    UI_S_M_prog_on_pause_max_await_ss = new UIValueSetter(UI_settings_master_limits_prog, true, 40, "Прогр.\nпростой? #АП", nullptr, &OpenSans_bold_14px);
    UI_S_M_prog_on_pause_max_await_ss->set_extra_button_logic({
        [this]() { var_prog_on_pause_max_await_ss.set(var_prog_on_pause_max_await_ss.get() + 10); },
        [this]() { var_prog_on_pause_max_await_ss.set(var_prog_on_pause_max_await_ss.get() - 10); },
        []() {}
    });
    UI_S_M_prog_on_pause_max_await_ss->add_ui_context_action([this]() {
        static char buffer[14];
        sprintf(buffer, "%01d:%02dс.", var_prog_on_pause_max_await_ss.get() / 60, var_prog_on_pause_max_await_ss.get() % 60);
        UI_S_M_prog_on_pause_max_await_ss->set_value(std::string(buffer));
    });

    UI_S_M_prog_await_spite_of_already_runned_ss = new UIValueSetter(UI_settings_master_limits_prog, true, 40, "Авто-пуск\nожидание", nullptr, &OpenSans_bold_14px);
    UI_S_M_prog_await_spite_of_already_runned_ss->set_extra_button_logic({
        [this]() { var_prog_await_spite_of_already_runned_ss.set(var_prog_await_spite_of_already_runned_ss.get() + 10); },
        [this]() { var_prog_await_spite_of_already_runned_ss.set(var_prog_await_spite_of_already_runned_ss.get() - 10); },
        []() {}
    });
    UI_S_M_prog_await_spite_of_already_runned_ss->add_ui_context_action([this]() {
        static char buffer[14];
        sprintf(buffer, "%01d:%02dс.", var_prog_await_spite_of_already_runned_ss.get() / 60, var_prog_await_spite_of_already_runned_ss.get() % 60);
        UI_S_M_prog_await_spite_of_already_runned_ss->set_value(std::string(buffer));
    });

    UI_S_M_prog_limit_heat_tempC_max = new UIValueSetter(UI_settings_master_limits_prog, false, 40, "Продукт °C\nmax Нагрев", nullptr, &OpenSans_bold_14px);
    UI_S_M_prog_limit_heat_tempC_max->set_extra_button_logic({
        [this]() { uint8_t var = var_prog_limit_heat_tempC_max.get() + 1;  var_prog_limit_heat_tempC_max.set(var > LIMIT_WATER_BOILING_POINT_TEMPC ? LIMIT_WATER_BOILING_POINT_TEMPC : var); },
        [this]() { uint8_t var = var_prog_limit_heat_tempC_max.get() - 1;  var_prog_limit_heat_tempC_max.set(var > LIMIT_WATER_BOILING_POINT_TEMPC ? LIMIT_WATER_BOILING_POINT_TEMPC : var); },
        []() {}
    });
    UI_S_M_prog_limit_heat_tempC_max->add_ui_context_action([this]() { UI_S_M_prog_limit_heat_tempC_max->set_value(var_prog_limit_heat_tempC_max.get(), "°C"); });

    UI_S_M_prog_limit_heat_tempC_min = new UIValueSetter(UI_settings_master_limits_prog, false, 40, "Продукт °C\nmin Нагрев", nullptr, &OpenSans_bold_14px);
    UI_S_M_prog_limit_heat_tempC_min->set_extra_button_logic({
        [this]() { uint8_t var = var_prog_limit_heat_tempC_min.get() + 1;  var_prog_limit_heat_tempC_min.set(var > LIMIT_WATER_BOILING_POINT_TEMPC ? LIMIT_WATER_BOILING_POINT_TEMPC : var); },
        [this]() { uint8_t var = var_prog_limit_heat_tempC_min.get() - 1;  var_prog_limit_heat_tempC_min.set(var > LIMIT_WATER_BOILING_POINT_TEMPC ? LIMIT_WATER_BOILING_POINT_TEMPC : var); },
        []() {}
    });
    UI_S_M_prog_limit_heat_tempC_min->add_ui_context_action([this]() { UI_S_M_prog_limit_heat_tempC_min->set_value(var_prog_limit_heat_tempC_min.get(), "°C"); });

    UI_S_M_prog_limit_chill_tempC_max = new UIValueSetter(UI_settings_master_limits_prog, false, 40, "Продукт °C\nmax Охлажд.", nullptr, &OpenSans_bold_14px);
    UI_S_M_prog_limit_chill_tempC_max->set_extra_button_logic({
        [this]() { uint8_t var = var_prog_limit_chill_tempC_max.get() + 1; var_prog_limit_chill_tempC_max.set(var > LIMIT_WATER_BOILING_POINT_TEMPC ? LIMIT_WATER_BOILING_POINT_TEMPC : var); },
        [this]() { uint8_t var = var_prog_limit_chill_tempC_max.get() - 1; var_prog_limit_chill_tempC_max.set(var > LIMIT_WATER_BOILING_POINT_TEMPC ? LIMIT_WATER_BOILING_POINT_TEMPC : var); },
        []() {}
    });
    UI_S_M_prog_limit_chill_tempC_max->add_ui_context_action([this]() { UI_S_M_prog_limit_chill_tempC_max->set_value(var_prog_limit_chill_tempC_max.get(), "°C"); });

    UI_S_M_prog_limit_chill_tempC_min = new UIValueSetter(UI_settings_master_limits_prog, false, 40, "Продукт °C\nmin Охлажд.", nullptr, &OpenSans_bold_14px);
    UI_S_M_prog_limit_chill_tempC_min->set_extra_button_logic({
        [this]() { uint8_t var = var_prog_limit_chill_tempC_min.get() + 1; var_prog_limit_chill_tempC_min.set(var > LIMIT_WATER_BOILING_POINT_TEMPC ? LIMIT_WATER_BOILING_POINT_TEMPC : var); },
        [this]() { uint8_t var = var_prog_limit_chill_tempC_min.get() - 1; var_prog_limit_chill_tempC_min.set(var > LIMIT_WATER_BOILING_POINT_TEMPC ? LIMIT_WATER_BOILING_POINT_TEMPC : var); },
        []() {}
    });
    UI_S_M_prog_limit_chill_tempC_min->add_ui_context_action([this]() { UI_S_M_prog_limit_chill_tempC_min->set_value(var_prog_limit_chill_tempC_min.get(), "°C"); });

    UI_S_M_prog_any_step_max_durat_ss = new UIValueSetter(UI_settings_master_limits_prog, false, 40, "Этап прогр.\nmax длит.", nullptr, &OpenSans_bold_14px);
    UI_S_M_prog_any_step_max_durat_ss->set_extra_button_logic({
        [this]() { var_prog_any_step_max_durat_ss.set(var_prog_any_step_max_durat_ss.get() + 5); },
        [this]() { var_prog_any_step_max_durat_ss.set(var_prog_any_step_max_durat_ss.get() - 5); },
        []() {}
    });
    UI_S_M_prog_any_step_max_durat_ss->add_ui_context_action([this]() {
        static char buffer[14];
        sprintf(buffer, "%01d:%02dс.", var_prog_any_step_max_durat_ss.get() / 60, var_prog_any_step_max_durat_ss.get() % 60);
        UI_S_M_prog_any_step_max_durat_ss->set_value(std::string(buffer));
    });

    UI_S_M_prog_any_step_min_durat_ss = new UIValueSetter(UI_settings_master_limits_prog, false, 40, "Этап прогр.\nmin длит.", nullptr, &OpenSans_bold_14px);
    UI_S_M_prog_any_step_min_durat_ss->set_extra_button_logic({
        [this]() { var_prog_any_step_min_durat_ss.set(var_prog_any_step_min_durat_ss.get() + 5); },
        [this]() { var_prog_any_step_min_durat_ss.set(var_prog_any_step_min_durat_ss.get() - 5); },
        []() {}
    });
    UI_S_M_prog_any_step_min_durat_ss->add_ui_context_action([this]() {
        static char buffer[14];
        sprintf(buffer, "%01d:%02dс.", var_prog_any_step_min_durat_ss.get() / 60, var_prog_any_step_min_durat_ss.get() % 60);
        UI_S_M_prog_any_step_min_durat_ss->set_value(std::string(buffer));
    });

    UI_S_M_prog_heaters_toggle_delay_ss = new UIValueSetter(UI_settings_master_limits_prog, false, 40, "ТЭН-ы\nзадерж. вкл", nullptr, &OpenSans_bold_14px);
    UI_S_M_prog_heaters_toggle_delay_ss->set_extra_button_logic({
        [this]() { var_prog_heaters_toggle_delay_ss.set(var_prog_heaters_toggle_delay_ss.get() + 1); },
        [this]() { var_prog_heaters_toggle_delay_ss.set(var_prog_heaters_toggle_delay_ss.get() - 1); },
        []() {}
    });
    UI_S_M_prog_heaters_toggle_delay_ss->add_ui_context_action([this]() { UI_S_M_prog_heaters_toggle_delay_ss->set_value(var_prog_heaters_toggle_delay_ss.get(), " сек."); });

    UI_S_M_prog_wJacket_toggle_delay_ss = new UIValueSetter(UI_settings_master_limits_prog, false, 40, "Охлаждение\nзадерж. вкл", nullptr, &OpenSans_bold_14px);
    UI_S_M_prog_wJacket_toggle_delay_ss->set_extra_button_logic({
        [this]() { var_prog_wJacket_toggle_delay_ss.set(var_prog_wJacket_toggle_delay_ss.get() + 1); },
        [this]() { var_prog_wJacket_toggle_delay_ss.set(var_prog_wJacket_toggle_delay_ss.get() - 1); },
        []() {}
    });
    UI_S_M_prog_wJacket_toggle_delay_ss->add_ui_context_action([this]() { UI_S_M_prog_wJacket_toggle_delay_ss->set_value(var_prog_wJacket_toggle_delay_ss.get(), " сек."); });


    /* master, page - 1 */
    UI_S_M_type_of_equipment_enum               ->set_position(0, 10, 40, 85);       
    UI_S_M_is_blowgun_by_rf                     ->set_position(0, 100, 40, 85);       
    UI_S_M_is_asyncM_rpm_float                  ->set_position(0, 190, 40, 85);
    UI_S_M_plc_language                         ->set_position(0, 10, 140, 175);   

    /* master, page - 2 */    
    UI_S_M_sensor_batt_min_V                    ->set_position(0, 10, 40, 40);
    UI_S_M_sensor_batt_max_V                    ->set_position(0, 55, 40, 40);
    UI_S_M_sensor_batt_V_min_12bit              ->set_position(0, 100, 40, 40);
    UI_S_M_sensor_batt_V_max_12bit              ->set_position(0, 145, 40, 40);
    UI_S_M_sensor_tempC_limit_4ma_12bit         ->set_position(0, 190, 40, 40);
    UI_S_M_sensor_tempC_limit_20ma_12bit        ->set_position(0, 235, 40, 40);
    UI_S_M_sensor_tempC_limit_4ma_degrees_C     ->set_position(0, 10, 140, 40);       
    UI_S_M_sensor_tempC_limit_20ma_degrees_C    ->set_position(0, 55, 140, 40);      
    UI_S_M_sensor_dac_rpm_limit_min_12bit       ->set_position(0, 100, 140, 40);        
    UI_S_M_sensor_dac_rpm_limit_max_12bit       ->set_position(0, 145, 140, 40);        
    UI_S_M_sensor_dac_asyncM_rpm_min            ->set_position(0, 190, 140, 40);              
    UI_S_M_sensor_dac_asyncM_rpm_max            ->set_position(0, 235, 140, 40);

    /* master, page - 3 */  
    UI_S_M_blowing_await_ss                     ->set_position(0, 10, 40, 85);                       
    UI_S_M_blowing_pump_power_lm                ->set_position(0, 100, 40, 85);                    
    UI_S_M_blowing_limit_ml_max                 ->set_position(0, 190, 40, 85);                  
    UI_S_M_blowing_limit_ml_min                 ->set_position(0, 10, 140, 85);                  
    UI_S_M_blowing_limit_ss_max                 ->set_position(0, 100, 140, 85);                  
    UI_S_M_blowing_limit_ss_min                 ->set_position(0, 190, 140, 85);
    
    /* master, page - 4 */ 
    UI_S_M_wJacket_tempC_limit_max              ->set_position(0, 10, 80, 65);                
    UI_S_M_prog_wJacket_drain_max_ss            ->set_position(0, 80, 80, 65);              
    UI_S_M_prog_on_pause_max_await_ss           ->set_position(0, 150, 80, 65);            
    UI_S_M_prog_await_spite_of_already_runned_ss->set_position(0, 220, 80, 65); 
    UI_S_M_prog_limit_heat_tempC_max            ->set_position(0, 10, 180, 65);              
    UI_S_M_prog_limit_heat_tempC_min            ->set_position(0, 80, 180, 65);              
    UI_S_M_prog_limit_chill_tempC_max           ->set_position(0, 150, 180, 65);             
    UI_S_M_prog_limit_chill_tempC_min           ->set_position(0, 220, 180, 65);             
    UI_S_M_prog_any_step_max_durat_ss           ->set_position(0, 10, 280, 65);            
    UI_S_M_prog_any_step_min_durat_ss           ->set_position(0, 80, 280, 65);            
    UI_S_M_prog_heaters_toggle_delay_ss         ->set_position(0, 150, 280, 65);           
    UI_S_M_prog_wJacket_toggle_delay_ss         ->set_position(0, 220, 280, 65);           
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

void UIService::display_wd_in_setters()
{
    UI_setter_wd_hh ->set_value(prog_tmpe_templates_wd_time->at(get_template_wd_index())->get().get_hours());
    UI_setter_wd_mm ->set_value(prog_tmpe_templates_wd_time->at(get_template_wd_index())->get().get_minutes());
    UI_setter_wd_ss ->set_value(prog_tmpe_templates_wd_time->at(get_template_wd_index())->get().get_seconds());
}