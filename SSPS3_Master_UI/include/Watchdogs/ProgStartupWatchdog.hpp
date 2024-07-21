#pragma once
#ifndef ProgStartupWatchdog_hpp
#define ProgStartupWatchdog_hpp

#include <Arduino.h>
#include "../FRAM_DB.hpp"
#include "UIService.hpp"

using namespace std;

extern S_DateTime * dt_rt;
extern UIService * UI_service;

class ProgStartupWatchdog
{
private:
    uint16_t time_span_ss_await_spite_of_already_runned_prog;
    bool ui_task_control_was_inited = false;

public:
    ProgStartupWatchdog(uint16_t var_prog_await_spite_of_already_runned_ss) : 
    time_span_ss_await_spite_of_already_runned_prog(var_prog_await_spite_of_already_runned_ss)
    {}

    void do_control()
    {
        static EquipmentType equipment_type;
        equipment_type = var_type_of_equipment_enum.get();

        bool is_pasteur_prog =
            equipment_type == EquipmentType::Pasteurizer ||
            equipment_type == EquipmentType::DairyTaxiPasteurizer ||
            equipment_type == EquipmentType::DairyTaxiPasteurizerFlowgun;

        if (is_pasteur_prog)
        {
            for (uint8_t auto_prog_i = 0; auto_prog_i < 3; auto_prog_i++)
            {
                static S_DateTime _dt_comparator;
                _dt_comparator.set_date(*dt_rt->get_date());
                _dt_comparator.set_time(prog_tmpe_templates_wd_time->at(auto_prog_i)->get());

                int32_t _time_span_ss = dt_rt->difference_in_seconds(_dt_comparator);

                bool _was_successfully_runned = false;
                bool _on_off = prog_tmpe_templates_wd_state->at(auto_prog_i)->get().on_off;
                bool _executed = prog_tmpe_templates_wd_state->at(auto_prog_i)->get().executed;
                bool _can_be_runned =
                    _time_span_ss > 0
                    && _on_off
                    && _executed;
                
                if (_can_be_runned)
                {
                    if (_time_span_ss < time_span_ss_await_spite_of_already_runned_prog)
                    {
                        _was_successfully_runned = start_program(var_type_of_equipment_enum.get(), 3 + auto_prog_i); // 0-2 main (not atuo prog). auto prog start at 3

                        if (_was_successfully_runned)
                        {
                            prog_tmpe_templates_wd_state->at(auto_prog_i)->ptr()->executed = true;
                            prog_tmpe_templates_wd_state->at(auto_prog_i)->accept();
                        }

                        break;
                    }
                    else
                    {
                        prog_tmpe_templates_wd_state->at(auto_prog_i)->ptr()->executed = true;
                        prog_tmpe_templates_wd_state->at(auto_prog_i)->accept();
                    }
                }
            }
        }
    }

    bool start_program(EquipmentType equipment_type, uint8_t prog_index)
    {
        if (prog_runned.get().is_runned)
            return false;

        uint8_t step_index = 0;

        add_new_task_step(step_index++, ProgramStepAimEnum::WATER_JACKET, 0, 0, 0, true, false, false);

        bool is_pasteur_prog =
            equipment_type == EquipmentType::Pasteurizer ||
            equipment_type == EquipmentType::DairyTaxiPasteurizer ||
            equipment_type == EquipmentType::DairyTaxiPasteurizerFlowgun;
        
        bool is_chzm_prog = equipment_type == EquipmentType::Cheesemaker;

        /* prog steps init */
        if (is_pasteur_prog || is_chzm_prog)
        {
            uint8_t local_steps = 0;

            if (is_pasteur_prog)
            {
                prog_index = min((uint8_t)(prog_tmpe_templates->size() - 1), prog_index);
                local_steps = 3;
            }
            else if (is_chzm_prog)
            {
                prog_index = min((uint8_t)(prog_chm_templates->size() - 1), prog_index);
                local_steps = 6;
            }

            for (uint8_t local = 0; local < local_steps; local++)
            {                
                if ((is_chzm_prog && prog_chm_templates->at(prog_index)->ptr()->get_step(local)->step_is_turned_on) ||
                    (is_pasteur_prog && prog_tmpe_templates->at(prog_index)->ptr()->get_step(local)->step_is_turned_on))
                {
                    ProgramStep * source;

                    if (is_chzm_prog)
                        source = prog_chm_templates->at(prog_index)->ptr()->get_step(local);
                    else
                        source = prog_tmpe_templates->at(prog_index)->ptr()->get_step(local);
                    
                    add_new_task_step(
                        step_index++,
                        source->aim == ProgramStepAimEnum::PASTEUR ? ProgramStepAimEnum::HEATING : source->aim,
                        source->fan,
                        source->tempC,
                        0,
                        true,
                        source->aim == ProgramStepAimEnum::CHILLING,
                        false
                    );

                    if (source->duration_ss > 0)
                        add_new_task_step(
                            step_index++,
                            source->aim == ProgramStepAimEnum::PASTEUR ? ProgramStepAimEnum::PASTEUR : ProgramStepAimEnum::EXPOSURE,
                            source->fan,
                            source->tempC,
                            source->duration_ss,
                            false,
                            source->aim == ProgramStepAimEnum::CHILLING,
                            false
                        );

                    if (source->await_ok_button)
                        add_new_task_step(
                            step_index++,
                            ProgramStepAimEnum::USER_AWAIT,
                            source->fan,
                            source->tempC,
                            0,
                            false,
                            source->aim == ProgramStepAimEnum::CHILLING,
                            true
                        );
                }
            }
        }

        /* saving prog steps count */
        prog_runned_steps_count.set(step_index);

        /* prog controller init */
        ProgramAimEnum _task_aim = static_cast<ProgramAimEnum>(
            is_pasteur_prog
            ? 1 + prog_index
            : static_cast<uint8_t>(ProgramAimEnum::CHM_MAIN_1) + prog_index
        );

        prog_runned.ptr()->start_task(_task_aim, var_prog_on_pause_max_await_ss.get());
        prog_runned.accept();

        fill_ui_task_contol(true);

        return true;
    }

    bool fill_ui_task_contol(bool reset_existed_history = false)
    {
        if (ui_task_control_was_inited && !reset_existed_history)
            return false;
        ui_task_control_was_inited = true;

        if (prog_runned.ptr()->is_runned)
        {
            for (uint16_t i = 0; i < prog_runned_steps_count.get(); i++)
            {
                ProgramStep* step = prog_runned_steps->at(i)->ptr();

                UITaskListItem* ui_step = UI_service->UI_task_roadmap_control->add_task_step(i == 0);
                ui_step->set_extra_button_logic({
                    [=](){ prog_runned_steps->at(i)->ptr()->fan++;              prog_runned_steps->at(i)->accept(); },
                    [=](){ prog_runned_steps->at(i)->ptr()->fan--;              prog_runned_steps->at(i)->accept(); },
                    [=](){ prog_runned_steps->at(i)->ptr()->tempC++;            prog_runned_steps->at(i)->accept(); },
                    [=](){ prog_runned_steps->at(i)->ptr()->tempC--;            prog_runned_steps->at(i)->accept(); },
                    [=](){ prog_runned_steps->at(i)->ptr()->duration_ss += 10;  prog_runned_steps->at(i)->accept(); },
                    [=](){ prog_runned_steps->at(i)->ptr()->duration_ss -= 10;  prog_runned_steps->at(i)->accept(); },
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
                    case ProgramStepAimEnum::EXPOSURE:      ui_step->set_step_name("Выдержка"); break;
                    case ProgramStepAimEnum::USER_AWAIT:    ui_step->set_step_name("Ожидание: нажмите кнопку"); break;

                    default:
                        break;
                    }
                });
                ui_step->add_ui_context_action([=]()
                {
                    ui_step->set_step_values(
                        prog_runned_steps->at(i)->get().fan,
                        prog_runned_steps->at(i)->get().tempC,
                        prog_runned_steps->at(i)->get().time_left_ss(),
                        prog_runned_steps->at(i)->get().state
                    );
                    // here add method call which save prog step into FRAM if this step is running (in vector by index)
                });
            } 

            UI_service->UI_task_roadmap_control->update_ui_base();
            UI_service->UI_task_roadmap_control->update_ui_context();
        }

        return true;
    }

private:
    void add_new_task_step(uint8_t new_step_index, ProgramStepAimEnum aim, uint8_t fan, uint8_t tempC, uint32_t duration_ss, bool untill_condition_met, bool is_cooling_step, bool await_ok) {
        prog_runned_steps->at(new_step_index)->set(ProgramStep(aim, fan, tempC, duration_ss, untill_condition_met, is_cooling_step, await_ok, 1));
    }
};

#endif