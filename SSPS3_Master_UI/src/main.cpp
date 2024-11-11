#include "../include/main.hpp"

UINotificationBar * UI_notification_bar;

FilterValue * filter_tempC_product;
FilterValue * filter_tempC_wJacket;
FilterValue * filter_24v_batt;

uint8_t FRAM_address = FRAM_I2C_ADDR;
TwoWire         * itcw;
DS3231          * rtc;
S_DateTime      * dt_rt;
STM32_slave     * STM32;

BlowingControl  * Blowing_control;

AsynchronousMotorWatchdog   * async_motor_wd;
ChillingWatchdog            * chilling_wd;
HeatingWatchdog             * heating_wd;
V380SupplyWatchdog          * v380_supply_wd;
WaterJacketDrainWatchdog    * wJacket_drain_wd;
ProgStartupWatchdog         * prog_stasrtup_wd;
bool prog_wd_first_call     = true;

UIService       * UI_service; 
UIManager       * UI_manager;

uint8_t     OptIn_state[8]  = { 0 };
uint16_t    AnIn_state[4]   = { 0 };
boolean     Pressed_key_accept_for_prog = false;

void blowing_proc(bool pistol_trigger);
void rtc_recovery_by_FRAM();
void setup_filters();
bool setup_UI(bool is_x = false);
void setup_controllers();
void setup_task_manager();
void setup_watchdogs();

void setup()
{
    Serial.begin(115200);

    pinMode(INT, INPUT_PULLDOWN);
    attachInterrupt(digitalPinToInterrupt(INT), interrupt_action, CHANGE);

    itcw    = new TwoWire(0);
    itcw    ->begin(SDA, SCL, 400000);
    STM32   = new STM32_slave(STM_I2C_ADDR);

    if (var_startup_key.get() != startup_key)
        Storage::reset_all(true);

    prod_time_x_cnt                         .set_is_system_val();
    var_type_of_equipment_enum              .set_is_system_val();
    var_stop_btn_type                       .set_is_system_val();
    var_sensor_tempC_limit_4ma_12bit        .set_is_system_val();
    var_sensor_tempC_limit_20ma_12bit       .set_is_system_val();
    var_sensor_tempC_limit_4ma_degrees_C    .set_is_system_val();
    var_sensor_tempC_limit_20ma_degrees_C   .set_is_system_val();
    var_type_of_equipment_enum.get();
    var_stop_btn_type.get();

#ifdef SSPS3_IS_CHEAP_SOLUTION_YES
    if (prod_time_x_cnt.get() >= TIME_X_SS)
    {
        setup_UI(true);
        UI_service->UI_date_time->hide_ui_hierarchy();
        UI_service->UI_machine_state_bar->hide_ui_hierarchy();
        UI_service->UI_notification_bar->hide_ui_hierarchy();
        UI_service->UI_notify_bar->hide_ui_hierarchy();

        lv_task_handler();
        delay(500);
        esp_restart();
    }
    prod_time_x_cnt.set(prod_time_x_cnt.local() + 1);
#endif

    Translator::set_lang(var_plc_language.get());

    rtc     = new DS3231(*itcw);
    dt_rt   = new S_DateTime(0, 0, 0, 0, 0, 0);
    dt_rt   ->set_rt_lambda([]()
    {
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
    rtc_recovery_by_FRAM();

    setup_filters();
    setup_UI();
    setup_controllers();
    setup_task_manager();
    setup_watchdogs();

    read_digital_signals();
    read_analog_signals(true);
    rt_task_manager.execute_task("task_do_programm");

    dt_rt->get_rt();

    if (false)
    {
        STM32->set(COMM_SET::RELAY, REL_ASYNC_M, 1);
        delay(500);
        STM32->set(COMM_SET::RELAY, REL_ASYNC_M, 0);
        delay(500);
        STM32->set(COMM_SET::RELAY, REL_ASYNC_M, 1);
        delay(500);
        STM32->set(COMM_SET::RELAY, REL_ASYNC_M, 0);
        delay(500);
        STM32->set(COMM_SET::RELAY, REL_ASYNC_M, 1);
        delay(500);
        STM32->set(COMM_SET::RELAY, REL_ASYNC_M, 0);
        delay(500);
    }
}

void loop()
{
    Blowing_control->do_blowing();

    if (interrupted_by_slave)
    {
        interrupted_by_slave = false;
        read_digital_signals();

        UI_service->UI_notification_bar->key_press(Pressed_key);
        UI_manager->handle_key_press(Pressed_key);

        if (!prog_runned.local().is_runned && UI_manager->is_current_control(ScreenType::PROGRAM_SELECTOR) && UI_service->get_is_pasteurizer())
        {
            switch (static_cast<KeyMap>(Pressed_key))
            {
            case KeyMap::L_STACK_4: prog_stasrtup_wd->start_program(var_type_of_equipment_enum.local(), 0); break ;
            case KeyMap::L_STACK_3: prog_stasrtup_wd->start_program(var_type_of_equipment_enum.local(), 1); break ;
            case KeyMap::L_STACK_2: prog_stasrtup_wd->start_program(var_type_of_equipment_enum.local(), 2); break ;
            default:
                break;
            }
        }

        if (prog_runned.local().is_runned)
        {
            Pressed_key_accept_for_prog = Pressed_key == static_cast<uint8_t>(KeyMap::LEFT_BOT);

            if (OptIn_state[DIN_ASYNC_M_ERROR] || OptIn_state[DIN_STOP_SENS])
            {
                heating_wd->emergency_stop();

                prog_runned.ptr()->end_task_by_user();
                prog_runned.accept();

                UI_service->UI_notification_bar->push_info(
                    OptIn_state[DIN_ASYNC_M_ERROR] ?
                    SystemNotification::ERROR_3_PHASE_MOTOR_IS_BROKEN :
                    SystemNotification::INFO_TASK_CANCELED_BY_USER /* like if OptIn_state[DIN_STOP_SENS] is 1*/
                );
            }
        }

        if (UI_manager->is_current_control(ScreenType::BLOWING_CONTROL))
            blowing_proc(OptIn_state[DIN_BLOWGUN_SENS]);
        else if (Blowing_control->is_runned)
            Blowing_control->blowgun_stop();
    }

    rt_task_manager.run();
    lv_task_handler();
}

void blowing_proc(bool pistol_trigger)
{
    if (Blowing_control->triggered_by != BlowingTriggerType::KEYBOARD)
        if (UI_service->UI_blowing_control->get_focused_index() >= 0)
        {
            uint8_t blow_index = min(UI_service->UI_blowing_control->get_focused_index(), (int16_t)3);
            BlowgunValue val = blowing_vals->at(blow_index)->local();

            Blowing_control->blowgun_trigger(pistol_trigger, false, var_blow_pump_calibration_lm.local(), blow_index, val);
        }
}

void rtc_recovery_by_FRAM()
{
    if (*dt_rt < var_last_rt.get())
    {
        S_DateTime saved_rt = var_last_rt.get();

        int year = saved_rt.get_date()->get_year() - 2000; 
        rtc->setHour(saved_rt.get_time()->get_hours());
        rtc->setMinute(saved_rt.get_time()->get_minutes());
        rtc->setSecond(saved_rt.get_time()->get_seconds());
        rtc->setDate(saved_rt.get_date()->get_day());
        rtc->setMonth(saved_rt.get_date()->get_month());
        rtc->setYear(year < 0 ? 0 : year);

        dt_rt->get_rt();
    }
}

void setup_filters()
{
    filter_tempC_product    = new FilterValue(
        &exp_filter_tempC_product,
        var_sensor_tempC_limit_4ma_12bit.get(),
        var_sensor_tempC_limit_20ma_12bit.get(),
        var_sensor_tempC_limit_4ma_degrees_C.get(),
        var_sensor_tempC_limit_20ma_degrees_C.get()
    );

    filter_tempC_wJacket    = new FilterValue(
        &exp_filter_tempC_wJacket,
        var_sensor_tempC_limit_4ma_12bit.get(),
        var_sensor_tempC_limit_20ma_12bit.get(),
        var_sensor_tempC_limit_4ma_degrees_C.get(),
        var_sensor_tempC_limit_20ma_degrees_C.get()
    );

    filter_24v_batt         = new FilterValue(
        &exp_filter_24v_batt,
        var_sensor_batt_V_min_12bit.get(),
        var_sensor_batt_V_max_12bit.get(),
        var_sensor_batt_min_V.get(),
        var_sensor_batt_max_V.get()
    );
}

bool setup_UI(bool is_x)
{
    UI_service = new UIService();
    
    if (is_x)
        return false;

    UI_manager = new UIManager();

    UI_manager->add_control(ScreenType::PROGRAM_SELECTOR,   UI_service->UI_prog_selector_control);
#ifndef SSPS3_IS_CHEAP_SOLUTION_YES
    UI_manager->add_control(ScreenType::TASK_ROADMAP,       UI_service->UI_task_roadmap_control);
#else
    UI_manager->add_control(ScreenType::TASK_ROADMAP,       UI_service->UI_cheap_roadmap_control);
#endif
    UI_manager->add_control(ScreenType::BLOWING_CONTROL,    UI_service->UI_blowing_control);
    UI_manager->add_control(ScreenType::MENU_USER,          UI_service->UI_menu_list_user);
    UI_manager->add_control(ScreenType::MENU_MASTER,        UI_service->UI_menu_list_master);

    if (!prog_runned.get().is_runned)
        UI_manager->set_control(ScreenType::PROGRAM_SELECTOR);
    else
        UI_manager->set_control(ScreenType::TASK_ROADMAP);

    UI_notification_bar = UI_service->UI_notification_bar;

    return true;
}

void setup_controllers()
{
    Blowing_control = new BlowingControl(
        UI_service->UI_notification_bar,
        [](bool state) { STM32->set(COMM_SET::RELAY, REL_BLOWGUN_PUMP, state); },
        var_blowing_await_ss.get(),
        var_blowing_pump_power_lm.get()
    );

    Blowing_control->set_blowing_ui_callback(
        [](float ms_aim, float ms_gone, BlowingType type, float ml_per_ms) {
            UI_service->UI_blowing_control->set_blow_value(ms_aim, ms_gone, type, ml_per_ms);
        }
    );
}

void setup_task_manager()
{
    rt_task_manager.add_task("task_update_filters", [](){
        read_analog_signals();
        //filter_tempC_product->add_value(AnIn_state[ADC_TEMPC_PRODUCT]);
        //filter_tempC_wJacket->add_value(AnIn_state[ADC_TEMPC_WJACKET]);
        filter_24v_batt     ->add_value(AnIn_state[ADC_VOLTAGE_BATT]);
    }, 100);
    
    rt_task_manager.add_task("task_update_UI_state_bar", [](){
        UI_service->UI_machine_state_bar->control_set_values_state_bar(
            rt_out_speed_async_m,
            var_is_asyncM_rpm_float.local(),
            filter_tempC_product->get_physical_value(),
            filter_tempC_wJacket->get_physical_value(),
            var_equip_have_wJacket_tempC_sensor.local(),
            OptIn_state[DIN_WJACKET_SENS]
                ? (rt_out_state_wJacket ? WaterJacketStateEnum::COOLING : WaterJacketStateEnum::FILLED)
                : (rt_out_state_wJacket ? WaterJacketStateEnum::FILLING : WaterJacketStateEnum::EMPTY),
            filter_24v_batt->get_percentage_value(),
            OptIn_state[DIN_380V_SIGNAL]
                ? ChargeStateEnum::CHARGERING
                : ChargeStateEnum::STABLE
        );
    }, 500);

    rt_task_manager.add_task("task_update_RT_clock", [](){
        dt_rt->get_rt();
        UI_service->UI_date_time->control_set_values_date_time(
            dt_rt->get_time()->get_hours(),
            dt_rt->get_time()->get_minutes(),
            dt_rt->get_time()->get_seconds(),
            dt_rt->get_date()->get_day(),
            dt_rt->get_date()->get_month(),
            dt_rt->get_date()->get_year()
        );

        if (var_last_rt.local().get_date()->get_day() != dt_rt->get_date()->get_day())
            for (uint i = 0; i < 3; i++)
            {
                prog_tmpe_templates_wd_state->at(i)->ptr()->executed = false;
                prog_tmpe_templates_wd_state->at(i)->accept();
            }

        var_last_rt.set(*dt_rt);

#ifdef SSPS3_IS_CHEAP_SOLUTION_YES
    prod_time_x_cnt.set(prod_time_x_cnt.local() + 1);
#endif

    }, 1000);

    rt_task_manager.add_task("task_do_programm", []() {
        ProgramStep to_do = prog_runned.ptr()->do_task(prog_wd_first_call);
        prog_runned.accept();
        prog_wd_first_call = false;

        //Serial.println("*task*");

        ProgramStep prev_step = *prog_runned.ptr()->get_prev_step();
        
        if (to_do.step_is_turned_on && to_do.aim != ProgramStepAimEnum::WATER_JACKET)
        {
            async_motor_wd      ->set_async_motor_speed(to_do.fan);

            if (to_do.must_be_cooled)
            {
                //Serial.println("*chilling step*");

                chilling_wd     ->set_aim(to_do.tempC, filter_tempC_product->get_physical_value());
                heating_wd      ->set_aim(0, 0);
            }
            else
            {
                //Serial.println("*main step*");

                /*
                if ((to_do.aim == ProgramStepAimEnum::EXPOSURE && prev_step.aim == ProgramStepAimEnum::CHILLING))
                    //|| var_type_of_equipment_enum.local() == EquipmentType::Cheesemaker)
                {
                    
                    if (var_equip_have_wJacket_tempC_sensor.get())
                        chilling_wd ->set_aim(
                            to_do.tempC + 2,
                            filter_tempC_product->get_physical_value(),
                            filter_tempC_wJacket->get_physical_value()
                        );
                    else
                    

                   Serial.println("*chilling exposure subtask*");

                    chilling_wd ->set_aim(
                        to_do.tempC + 1,
                        filter_tempC_product->get_physical_value()
                    );
                }
                else
                {
                    
                    Serial.println("*without chilling*");
                    chilling_wd     ->set_aim(0, 0);
                }
                */

                //Serial.println("*without chilling*");
                chilling_wd     ->set_aim(0, 0);
                
                //Serial.println("*heating exposure subtask*");
                if (var_equip_have_wJacket_tempC_sensor.get())
                    heating_wd      ->set_aim(
                        to_do.tempC + 1,
                        filter_tempC_product->get_physical_value(),
                        filter_tempC_wJacket->get_physical_value()
                    );
                else
                    heating_wd  ->set_aim(
                        min(to_do.tempC, (uint8_t)(to_do.tempC - 1)),
                        filter_tempC_product->get_physical_value()
                    );
            }
        }
        else if (to_do.step_is_turned_on && to_do.aim == ProgramStepAimEnum::WATER_JACKET)
        {
            //Serial.println("*water jacket step*");
            async_motor_wd  ->set_async_motor_speed(to_do.fan);
            chilling_wd     ->set_water_intake();
        }
        else
        {
            //Serial.println("*awaiting*");
            async_motor_wd  ->set_async_motor_speed(0);
            chilling_wd     ->set_aim(0, 0);
            heating_wd      ->set_aim(0, 0);
        }
        
        //Serial.println("");

        wJacket_drain_wd    ->water_in_jacket(
            OptIn_state[DIN_WJACKET_SENS],
            prog_runned.local().is_runned,
            to_do.aim == ProgramStepAimEnum::WATER_JACKET
        );

#ifndef SSPS3_IS_CHEAP_SOLUTION_YES
        UI_service->UI_task_roadmap_control->update_task_steps_state();
        UI_service->UI_task_roadmap_control->update_ui_context();
#else
        UI_service->UI_cheap_roadmap_control->update_ui_context();
#endif
    }, 500);

    rt_task_manager.add_task("task_call_watchdogs", []() {
        chilling_wd         ->do_control();
        heating_wd          ->do_control();
        v380_supply_wd      ->do_control(OptIn_state[DIN_380V_SIGNAL]);
        wJacket_drain_wd    ->do_control();
    }, 200);

    rt_task_manager.add_task("task_do_another_lazy_things", []() {
        prog_stasrtup_wd->do_control(
            UI_manager->is_current_control(ScreenType::MENU_USER) ||
            UI_manager->is_current_control(ScreenType::MENU_MASTER)
        );

        if (filter_tempC_product->get_physical_value() >= 105 || filter_tempC_product->get_physical_value() < -10)
            UI_service->UI_notification_bar->push_info(SystemNotification::ERROR_TEMP_C_SENSOR_BROKEN);

        if (var_equip_have_wJacket_tempC_sensor.local())
            if (filter_tempC_wJacket->get_physical_value() >= 105 || filter_tempC_wJacket->get_physical_value() < -10)
                UI_service->UI_notification_bar->push_info(SystemNotification::ERROR_TEMP_C_SENSOR_BROKEN);
        
        if (prog_runned.local().is_runned && prog_runned.local().state == TaskStateEnum::RUNNED)
            UI_service->UI_notification_bar->display();
    }, 10000);
}

void setup_watchdogs()
{
    async_motor_wd      = new AsynchronousMotorWatchdog(
        [](bool state)      { STM32->set(COMM_SET::RELAY, REL_ASYNC_M, rt_out_state_async_m = state); },
        [](uint8_t speed_rpm)
        {
            STM32->set(
                COMM_SET::DAC,
                DAC_ASYNC_M_SPEED,
                var_sensor_dac_rpm_limit_max_12bit.get()
                    / var_sensor_dac_asyncM_rpm_max.get()
                    * speed_rpm
            );

            if (var_is_asyncM_rpm_float.local())
                rt_out_speed_async_m = speed_rpm;
            else
                rt_out_speed_async_m = rt_out_state_async_m
                    ? var_sensor_dac_asyncM_rpm_max.local()
                    : 0;
        },
        var_sensor_dac_asyncM_rpm_max.get(),
        var_is_asyncM_rpm_float.get()
    );

    chilling_wd         = new ChillingWatchdog(
        [](bool state)      { STM32->set(COMM_SET::RELAY, REL_WJACKET_VALVE, rt_out_state_wJacket = state); },
        var_prog_wJacket_toggle_delay_ss.get(),
        var_prog_coolign_water_safe_mode.get()
    );

    heating_wd          = new HeatingWatchdog(
        [](bool state)      { STM32->set(COMM_SET::RELAY, REL_HEATERS, rt_out_state_heaters = state); },
        var_prog_heaters_toggle_delay_ss.get(),
        var_wJacket_tempC_limit_max.get()
    );

    v380_supply_wd      = new V380SupplyWatchdog([](bool is_error){
        prog_runned.ptr()->pause_state_by_wd_380v(is_error);
        prog_runned.accept();

        if (prog_runned.local().is_runned && is_error)
            UI_service->UI_notification_bar->push_info(SystemNotification::WARNING_380V_NO_POWER);
    });

    wJacket_drain_wd    = new WaterJacketDrainWatchdog(
        [](bool state)      { STM32->set(COMM_SET::RELAY, REL_WJACKET_VALVE, rt_out_state_wJacket = state); },
        [](bool state)
        {
            prog_runned.ptr()->pause_state_by_wd_wJacket_drain(state);
            prog_runned.accept();

            if (state)
            {
                UI_service->UI_notification_bar->push_info(SystemNotification::WARNING_WATER_JACKET_NO_WATER);
            }
        },
        var_prog_wJacket_drain_max_ss.get()
    );

    prog_stasrtup_wd    = new ProgStartupWatchdog(UI_service->UI_task_roadmap_control, var_prog_await_spite_of_already_runned_ss.get());
    prog_stasrtup_wd->fill_ui_task_contol();
}