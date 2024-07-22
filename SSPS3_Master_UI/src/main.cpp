#include "../include/main.hpp"

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

void rtc_recovery_by_FRAM();
void setup_filters();
void setup_UI();
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

    //Storage::reset_all();   
    
    setup_filters();
    setup_UI();
    setup_controllers();
    setup_task_manager();
    setup_watchdogs();

    dt_rt->get_rt();
}

bool demo_was_runned = false;

void loop()
{
    Blowing_control->do_blowing();
    rt_task_manager.run();

    if (millis() > 10000 && !demo_was_runned)
    {
        demo_was_runned = true;
        prog_stasrtup_wd->start_program(EquipmentType::DairyTaxiPasteurizer, 0);
    }

    /* somewhere in ProgramControl */
    /*
    async_motor_wd->set_async_motor_speed(20);
    chilling_wd->get_aim(15, 50);
    heating_wd->get_aim(85, 50, 55);
    wJacket_drain_wd->water_in_jacket(true);
    */

    /* somewhere in loop */
    /*
    chilling_wd->do_control();
    heating_wd->do_control();
    v380_supply_wd->do_control(true);
    wJacket_drain_wd->do_control();
    */

    if (interrupted_by_slave)
    {
        interrupted_by_slave = false;
        read_input_signals();

        UI_service->UI_notification_bar->key_press(Pressed_key);
        
        // prog_selector control
        UI_service->UI_prog_selector_control->get_selected()->key_press(Pressed_key);
        UI_service->UI_prog_selector_control->get_selected(true)->key_press(Pressed_key);

        // task control
        //UI_service->UI_task_roadmap_control->get_selected()->key_press(Pressed_key);
        //UI_service->UI_task_roadmap_control->get_selected(true)->key_press(Pressed_key);

        // user settings control
        //if (UI_service->UI_menu_list_user->is_selected_on_child())
        //    UI_service->UI_menu_list_user->get_selected(true)->key_press(Pressed_key);
        //UI_service->UI_menu_list_user->get_selected()->key_press(Pressed_key);

        // master settings control
        //if (UI_service->UI_menu_list_master->is_selected_on_child())
        //    UI_service->UI_menu_list_master->get_selected(true)->key_press(Pressed_key);
        //UI_service->UI_menu_list_master->get_selected()->key_press(Pressed_key);

        // blowing control
        //UI_service->UI_blowing_control->get_selected()->key_press(Pressed_key);
        //UI_service->UI_blowing_control->get_selected(true)->key_press(Pressed_key);
    }

    lv_task_handler();
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

void setup_UI()
{
    UI_service = new UIService();
}

void setup_controllers()
{
    Blowing_control = new BlowingControl(
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
        filter_tempC_product->add_value(AnIn_state[ADC_TEMPC_PRODUCT]);
        filter_tempC_wJacket->add_value(AnIn_state[ADC_TEMPC_WJACKET]);
        filter_24v_batt     ->add_value(AnIn_state[ADC_VOLTAGE_BATT]);
    }, 1000);
    
    rt_task_manager.add_task("task_update_UI_state_bar", [](){
        UI_service->UI_machine_state_bar->control_set_values_state_bar(
            rt_out_speed_async_m,
            filter_tempC_product->get_physical_value(),
            OptIn_state[DIN_WJACKET_SENS] ?
                WaterJacketStateEnum::FILLED : (rt_out_state_wJacket ?
                    WaterJacketStateEnum::FILLING : WaterJacketStateEnum::EMPTY
                ),
            filter_24v_batt->get_percentage_value(),
            OptIn_state[DIN_380V_SIGNAL] ? ChargeStateEnum::CHARGERING : ChargeStateEnum::STABLE
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
        var_last_rt.set(*dt_rt);
    }, 1000);

    rt_task_manager.add_task("task_do_programm", []() {
        ProgramStep to_do = prog_runned.ptr()->do_task(prog_wd_first_call);
        prog_runned.accept();
        prog_wd_first_call = false;

        async_motor_wd      ->set_async_motor_speed(to_do.fan);
        chilling_wd         ->set_aim(to_do.must_be_cooled ? to_do.tempC : to_do.tempC + 2, filter_tempC_product->get_physical_value());
        if (var_equip_have_wJacket_tempC_sensor.get())
            heating_wd      ->set_aim(
                to_do.must_be_cooled ? to_do.tempC - 2: to_do.tempC,
                filter_tempC_product->get_physical_value(),
                filter_tempC_wJacket->get_physical_value()
            );
        else
            heating_wd      ->set_aim(
                to_do.must_be_cooled ? to_do.tempC - 2 : to_do.tempC,
                filter_tempC_product->get_physical_value()
            );
        wJacket_drain_wd    ->water_in_jacket(!OptIn_state[DIN_WJACKET_SENS]); ////////////////////////////////////////////////// !
    
        if (!to_do.step_is_turned_on)
        {
            async_motor_wd  ->set_async_motor_speed(0);
            chilling_wd     ->set_aim(0, 0);
            heating_wd      ->set_aim(0, 0);
            wJacket_drain_wd->water_in_jacket(true);
        }

        UI_service->UI_task_roadmap_control->update_task_steps_state();
        UI_service->UI_task_roadmap_control->update_ui_context();
    }, 500);

    rt_task_manager.add_task("task_call_watchdogs", []() {
        //if (!prog_runned.get().is_runned)
        //    async_motor_wd  ->set_async_motor_speed(0);
        
        chilling_wd         ->do_control();
        heating_wd          ->do_control();
        v380_supply_wd      ->do_control(!OptIn_state[DIN_380V_SIGNAL]); //////////////////////////////////////////////////////// !
        wJacket_drain_wd    ->do_control();
    }, 200);
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
            rt_out_speed_async_m = speed_rpm;
        },
        var_sensor_dac_asyncM_rpm_max.get()
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

    v380_supply_wd      = new V380SupplyWatchdog([](bool state){
        prog_runned.ptr()->pause_state_by_wd_380v(state);
        prog_runned.accept();
    });

    wJacket_drain_wd    = new WaterJacketDrainWatchdog(
        [](bool state)      { STM32->set(COMM_SET::RELAY, REL_WJACKET_VALVE, rt_out_state_wJacket = state); },
        [](bool state)
        {
            prog_runned.ptr()->pause_state_by_wd_wJacket_drain(state);
            prog_runned.accept();
        },
        var_prog_wJacket_drain_max_ss.get()
    );

    prog_stasrtup_wd    = new ProgStartupWatchdog(UI_service->UI_task_roadmap_control, var_prog_await_spite_of_already_runned_ss.get());
    prog_stasrtup_wd->fill_ui_task_contol();
}