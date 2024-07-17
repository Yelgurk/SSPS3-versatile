#include "../include/main.hpp"

uint8_t FRAM_address = FRAM_I2C_ADDR;
TwoWire         * itcw;
DS3231          * rtc;
S_DateTime      * dt_rt;
STM32_slave     * STM32;
UIService       * UI_service; 

lv_obj_t * label_timer;
vector<bool> tests_state        = { false, false, false, false, false };
vector<uint8_t> tests_durat_ss  = { 2, 5, 20, 15, 5 };
uint16_t timer_ss_current       = tests_durat_ss.at(0);
uint16_t tets_current           = 0;
uint16_t time_ss_to_reboot      = 5;

lv_color_t colors[] = {
    lv_color_hex(0x1C1C1C),
    lv_color_hex(0xB5B8B1),
    lv_color_hex(0xFF0019),
    lv_color_hex(0x24FF00),
    lv_color_hex(0x0029FF)
};
uint8_t color_index = 0;
lv_obj_t * rect;

void anim_color_cb(void * obj, int32_t color) {
    lv_obj_set_style_bg_color((lv_obj_t *)obj, lv_color_hex(color), 0);
}

// Callback функция для переключения цветовых переходов
void anim_color_ready_cb(lv_anim_t * a) {
    // Переход к следующему цвету в массиве
    color_index = (color_index + 1) % (sizeof(colors) / sizeof(colors[0]));

    // Установка новых значений для анимации
    lv_anim_set_values(a, lv_color_to_u32(colors[color_index]), lv_color_to_u32(colors[(color_index + 1) % (sizeof(colors) / sizeof(colors[0]))]));
    lv_anim_start(a);
}

void set_label_timer_val(uint16_t ss) {
    lv_label_set_text(label_timer, to_string(ss).c_str());
}

void setup()
{
    Serial.begin(115200);

    pinMode(INT, INPUT_PULLDOWN);
    attachInterrupt(digitalPinToInterrupt(INT), interrupt_action, CHANGE);

    delay(5000);

    itcw = new TwoWire(0);
    itcw->begin(SDA, SCL, 400000);

    rtc             = new DS3231(*itcw);
    dt_rt = new S_DateTime(0, 0, 0, 0, 0, 0);
    dt_rt->set_rt_lambda([]() {
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
    UI_service      = new UIService();

    rect = lv_obj_create(UI_service->screen);
    lv_obj_set_size(rect, 480, 320);
    lv_obj_center(rect);
    lv_obj_set_style_border_width(rect, 0, 0);
    lv_obj_set_style_radius(rect, 0, 0);

    static lv_anim_t a_color;
    lv_anim_init(&a_color);
    lv_anim_set_var(&a_color, rect);
    lv_anim_set_time(&a_color, 100000); // Продолжительность анимации 1 секунда
    lv_anim_set_exec_cb(&a_color, anim_color_cb);
    lv_anim_set_path_cb(&a_color, lv_anim_path_linear); // Установка линейного перехода
    lv_anim_set_ready_cb(&a_color, anim_color_ready_cb); // Callback для перехода к следующему цвету

    // Начало анимации цвета
    lv_anim_set_values(&a_color, lv_color_to_u32(colors[0]), lv_color_to_u32(colors[1]));
    lv_anim_start(&a_color);

    rt_task_manager.add_task("updateT_timer", [](){
        if (timer_ss_current <= 0)
        {
            tests_state[tets_current] = true;
            ++tets_current;
            timer_ss_current = tets_current >= tests_state.size() ? 0 : tests_durat_ss.at(tets_current);

            if (timer_ss_current == 0)
                --time_ss_to_reboot;

            if (time_ss_to_reboot <= 0)
                ESP.restart();
        }
        else
            timer_ss_current--;
            
        set_label_timer_val(timer_ss_current);
    }, 1000);

    rt_task_manager.add_task("test_0_fram", [](){
        if (tets_current == 0)
        {
            if (true)
            {
                Serial.print("is init:  ");
                Serial.print(mem_Timer1.crc_state());
                Serial.print(" | ");
                Serial.print(mem_Timer2.crc_state());
                Serial.print(" | ");
                Serial.print(mem_Timer3.crc_state());
                Serial.print(" | ");
                Serial.print(mem_Timer4.crc_state());
                Serial.print(" | ");
                Serial.print(mem_Timer5.crc_state());
                Serial.print(" | ");
                Serial.print(mem_String.crc_state());
                Serial.print(" | ");
                Serial.print(mem_TDS_1.crc_state());
                Serial.print(" | ");
                Serial.print(mem_DT.crc_state());
                Serial.println();
                if (!mem_Timer1.crc_state()) mem_Timer1.unset();
                if (!mem_Timer2.crc_state()) mem_Timer2.unset();
                if (!mem_Timer3.crc_state()) mem_Timer3.unset();
                if (!mem_Timer4.crc_state()) mem_Timer4.unset();
                if (!mem_Timer5.crc_state()) mem_Timer5.unset();
                if (!mem_String.crc_state()) mem_String.unset();
                if (!mem_TDS_1.crc_state()) mem_TDS_1.unset();
                if (!mem_TDS_2.crc_state()) mem_TDS_2.unset();
                if (!mem_TDS_4.crc_state()) mem_TDS_4.unset();
                if (!mem_DT.crc_state()) mem_DT.unset();
            }

            if (true)
            {
                Serial.print("read old: ");
                Serial.print(mem_Timer1);
                Serial.print(" | ");
                Serial.print(mem_Timer2);
                Serial.print(" | ");
                Serial.print(mem_Timer3);
                Serial.print(" | ");
                Serial.print(mem_Timer4);
                Serial.print(" | ");
                Serial.print(mem_Timer5);
                Serial.print(" | ");
                Serial.print(mem_String);
                Serial.print(" | ");
                Serial.print(mem_TDS_1.get().to_string().c_str());
                Serial.print(" | ");
                Serial.print(mem_DT.get().to_string().c_str());
                Serial.println();
            }

            if (true)
            {
                Serial.print("set:      ");
                mem_Timer1.set(!mem_Timer1.get());
                mem_Timer2.set(290 * multipl);
                mem_Timer3.set(290 * 10 * multipl);
                mem_Timer4.set(290 * 10 * 2 * multipl);
                mem_Timer5.set(290 * 10 * 4 * multipl);
                mem_String.set("inc = " + to_string(multipl));

                TaskDataStruct tds_edit = mem_TDS_1.get();
                tds_edit.done = !tds_edit.done;
                tds_edit.lap += 2;
                tds_edit.tempC += 5;
                tds_edit.ms_total += 10;
                tds_edit.ms_left += 20;

                Serial.print(mem_Timer1.get());
                Serial.print(" | ");
                Serial.print((uint8_t)(290 * multipl));
                Serial.print(" | ");
                Serial.print((uint16_t)(290 * 10 * multipl));
                Serial.print(" | ");
                Serial.print((uint32_t)(290 * 10 * 2 * multipl));
                Serial.print(" | ");
                Serial.print((uint32_t)(290 * 10 * 4 * multipl));
                Serial.print(" | ");
                Serial.print(("inc = " + to_string(multipl)).c_str());
                Serial.print(" | ");
                Serial.print(tds_edit.to_string().c_str());
                Serial.print(" | ");
                Serial.print((mem_DT.get() += S_Time(0, 0, 1)).to_string().c_str());
                Serial.println();

                mem_TDS_1.set(tds_edit);
                mem_DT.set(mem_DT.get() += S_Time(0, 0, 1));
            }

            if (true)
            {
                Serial.print("read new: ");
                Serial.print(mem_Timer1.get());
                Serial.print(" | ");
                Serial.print(mem_Timer2.get());
                Serial.print(" | ");
                Serial.print(mem_Timer3.get());
                Serial.print(" | ");
                Serial.print(mem_Timer4.get());
                Serial.print(" | ");
                Serial.print(mem_Timer5.get());
                Serial.print(" | ");
                Serial.print(mem_String.get().c_str());
                Serial.print(" | ");
                Serial.print(mem_TDS_1.get().to_string().c_str());
                Serial.print(" | ");
                Serial.print(mem_DT.get().to_string().c_str());

                Serial.println();
            }

            ++multipl;
            Serial.println();
        }
    }, 100);

    rt_task_manager.add_task("test_1_rtc_ds3231", []() {
        if (tets_current == 1)
        {
            dt_rt->get_rt();
            Serial.print(dt_rt->get_time()->get_hours());
            Serial.print(":");
            Serial.print(dt_rt->get_time()->get_minutes());
            Serial.print(":");
            Serial.print(dt_rt->get_time()->get_seconds());
            Serial.print(", ");
            Serial.print(dt_rt->get_date()->get_day());
            Serial.print(".");
            Serial.print(dt_rt->get_date()->get_month());
            Serial.print(".");
            Serial.println(dt_rt->get_date()->get_year());
        }
    }, 1000);

    rt_task_manager.add_task("test_2_kb", []() {
        if (tets_current == 2)
        {
            if (Pressed_key != static_cast<uint8_t>(KeyMap::_END))
                Serial.println(Pressed_key);
        }
    }, 50);

    rt_task_manager.add_task("test_3_input", []() {
        if (tets_current == 3)
        {
            Serial.print(OptIn_state[0] = STM32->get(COMM_GET::DGIN, 0));
            Serial.print(" ");
            Serial.print(OptIn_state[1] = STM32->get(COMM_GET::DGIN, 1));
            Serial.print(" ");
            Serial.print(OptIn_state[2] = STM32->get(COMM_GET::DGIN, 2));
            Serial.print(" ");
            Serial.print(OptIn_state[3] = STM32->get(COMM_GET::DGIN, 3));
            Serial.print(" ");
            Serial.print(OptIn_state[4] = STM32->get(COMM_GET::DGIN, 4));
            Serial.print(" ");
            Serial.print(OptIn_state[5] = STM32->get(COMM_GET::DGIN, 5));
            Serial.print(" ");
            Serial.print(OptIn_state[6] = STM32->get(COMM_GET::DGIN, 6));
            Serial.print(" ");
            Serial.print(OptIn_state[7] = STM32->get(COMM_GET::DGIN, 7));

            if (true)
            {
                Serial.print(" | ");

                Serial.print(STM32->get(COMM_GET::ANIN, 0));
                Serial.print(" ");
                Serial.print(STM32->get(COMM_GET::ANIN, 1));
                Serial.print(" ");
                Serial.print(STM32->get(COMM_GET::ANIN, 2));
                Serial.print(" ");
                Serial.print(STM32->get(COMM_GET::ANIN, 3));
            }
            
            Serial.println();
        }
    }, 100);

    rt_task_manager.add_task("test_4_output", []() {
        if (tets_current == 4)
        {
            Serial.println("Output test");

            dac_mult += 400;
            dac_mult = dac_mult < 4095 ? dac_mult : 4095;

            STM32->set(COMM_SET::RELAY, 0, !STM32->get(COMM_GET::RELAY, 0));
            STM32->set(COMM_SET::RELAY, 1, !STM32->get(COMM_GET::RELAY, 1));
            STM32->set(COMM_SET::RELAY, 2, !STM32->get(COMM_GET::RELAY, 2));
            STM32->set(COMM_SET::RELAY, 3, !STM32->get(COMM_GET::RELAY, 3));
            STM32->set(COMM_SET::RELAY, 4, !STM32->get(COMM_GET::RELAY, 4));
            STM32->set(COMM_SET::RELAY, 5, !STM32->get(COMM_GET::RELAY, 5));
            STM32->set(COMM_SET::RELAY, 6, !STM32->get(COMM_GET::RELAY, 6));
            STM32->set(COMM_SET::RELAY, 7, !STM32->get(COMM_GET::RELAY, 7));
            STM32->set(COMM_SET::DAC, 0, dac_mult);
        }
    }, 500);

    label_timer = lv_label_create(UI_service->screen);
    lv_obj_set_style_text_font(label_timer, &OpenSans_bold_24px, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(label_timer);

    set_label_timer_val(timer_ss_current);
}

void loop()
{
    rt_task_manager.run();

    if (interrupted_by_slave)
    {
        interrupted_by_slave = false;
        read_input_signals();
    }

    lv_task_handler();
}