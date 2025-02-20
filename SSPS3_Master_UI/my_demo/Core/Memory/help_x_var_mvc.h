#pragma once
#ifndef HELP_X_VAR_MVC_H

/*

void setup()
{
    custom_demo_2();
}

void loop()
{
    while (1)
    {
        if (XStorage->TestVar1++ % 100 == 0)
        {
            if (++XStorage->TestVar2 <= 100)
                XStorage->TestVar3 = XStorage->TestVar2;
            else
            if (XStorage->TestVar2 <= 200)
                XStorage->TestVar3 = 200 - XStorage->TestVar2;
            else
                XStorage->TestVar3 = XStorage->TestVar2 = 0;
        }

        uint32_t time_till_next = lv_timer_handler();
        lv_delay_ms(time_till_next);
    }
}

void custom_demo_2()
{
    XStorage->TestVar1 = 0;
    XStorage->TestVar2 = 0;
    XStorage->TestVar3 = 0;

    // XVarMVC_String
    // binding UI element to TestVar3 by "OnPropertyChange" mechanic via "mvc_string_bind(...)" + inited lambda for that through "XVarMVC_String::set_ui_label_notify_event()"
    XStorage->TestVar3.mvc_string_bind(
        GuiScreenTask::instance()
        ->get_control_header_bar()
        ->get_widget_speed_RPM()
        ->get_label()
    );

    // LVGL
    // create demo slider
    lv_obj_t* _demo_slider = lv_slider_create(GuiScreenTask::instance()->get_element_body());
    lv_obj_set_width(_demo_slider, 200);
    lv_obj_align(_demo_slider, LV_ALIGN_CENTER, 0, 0);
    lv_slider_set_range(_demo_slider, 0, 100);
    lv_slider_set_value(_demo_slider, XStorage->TestVar2, LV_ANIM_OFF);

    // XVarMVC_LVGL_V9
    // binding LVGL slider to TestVar3 via resolved "x_var_mvc_lvgl_v9.h" using LVGL lv_subject_t
    // when _value changed => lv_subject_t changed => slider state changed
    XStorage->TestVar3.mvc_lvgl_bind_one_way(
        _demo_slider,
        lv_slider_bind_value
    );

    // XVarMVC_Obj
    // goto: GUI->View->Component->ui_component_small_widget.h => "on_x_var_value_changed()" overriden for demo
    // binding UI element to TestVar3 via a call to internal override business logic of UI element "on_x_var_value_changed()", not via a generalised string output method
    XStorage->TestVar3._mvc_obj_bind(
        GuiScreenTask::instance()
        ->get_control_header_bar()
        ->get_widget_temp_C()
        ->get_observer()
    );

    // ui element one_way_to_source binding to XVar<T>
    // when slider moved => _value changed (via mvc_..._cb_event) => lv_subject_t changed
    XStorage->TestVar3.mvc_lvgl_bind_two_way(
        _demo_slider,
        lv_slider_bind_value
    );

    // one more way for label binding using XVarMVC_LVGL_V9 with text formatters
    XStorage->TestVar3.mvc_lvgl_bind_one_way(
        GuiScreenTask::instance()
        ->get_control_header_bar()
        ->get_widget_water_J()
        ->get_label(),
        lv_label_bind_text,
        "%d °C"
    );
}

*/

#endif