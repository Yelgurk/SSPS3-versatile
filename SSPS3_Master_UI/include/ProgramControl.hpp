#pragma once

#ifndef ProgramControl_hpp
#define ProgramControl_hpp

#include <Arduino.h>
#include "../../SSPS3_Master_Domain/include/DateTime/S_DateTime.hpp"
#include "../include/UIControls/UITaskRoadmapList.hpp"
#include "../include/UIControls/UINotificationBar.hpp"
#include "../../SSPS3_Master_Domain/include/FRAM/FRAM_Storage.hpp"
#include "../../SSPS3_Master_Domain/include/FRAM/FRAM_Object.hpp"
#include "../../SSPS3_Master_Domain/include/FRAM/FRAM_RW.hpp"
#include "STM32Pinouts.hpp"
#include "Filters/FilterValue.hpp"

enum class ProgramAimEnum : uint8_t
{
    NONE,
    TMP_PASTEUR,
    TMP_CHILL,
    TMP_HEAT,
    TMP_WATCHDOG_1,
    TMP_WATCHDOG_2,
    TMP_WATCHDOG_3,
    CHM_MAIN_1,
    CHM_MAIN_2,
    CHM_MAIN_3,
    CHM_TEMPL_1,
    CHM_TEMPL_2,
    CHM_TEMPL_3,
    CHM_TEMPL_4,
    CHM_TEMPL_5,
    CHM_TEMPL_6,
    CHM_TEMPL_7
};

enum class ProgramStepAimEnum : uint8_t
{
    WATER_JACKET,
    PASTEUR,
    CHILLING,
    CUTTING,
    MIXING,
    HEATING,
    DRYING,
    EXPOSURE,
    TO_TEMPERATURE,
    USER_AWAIT
};

template <typename T>
bool IsInBounds(const T& value, const T& low, const T& high) {
    return !(value < low) && (value < high);
}

struct __attribute__((packed)) ProgramStep
{
    ProgramStepAimEnum aim;
    uint8_t fan;
    uint8_t tempC;
    uint32_t duration_ss;
    bool untill_condition_met;
    bool must_be_cooled;
    bool await_ok_button;
    bool step_is_turned_on;

    StepStateEnum state = StepStateEnum::AWAIT;
    uint32_t gone_ss = 0;

    ProgramStep() : ProgramStep(ProgramStepAimEnum::TO_TEMPERATURE, 0, 0, 0, 0, 0, 0, 0) {}

    ProgramStep(
        ProgramStepAimEnum aim,
        uint8_t fan,
        uint8_t tempC,
        uint32_t duration_ss,
        bool untill_condition_met,
        bool must_be_cooled,
        bool await_ok_button,
        bool step_is_turned_on
    ) :
    aim(aim),
    fan(fan),
    tempC(tempC),
    duration_ss(duration_ss),
    untill_condition_met(untill_condition_met),
    must_be_cooled(must_be_cooled),
    await_ok_button(await_ok_button),
    step_is_turned_on(step_is_turned_on)
    {}

    bool is_time_out() {
        return gone_ss >= duration_ss;
    }

    void iteration_ss(uint32_t ss) {
        gone_ss += ss;
    }

    int32_t time_left_ss() {
        return duration_ss - gone_ss;
    }
};

extern UINotificationBar * UI_notification_bar;
extern S_DateTime * dt_rt;

extern FRAMObject<uint16_t>&                prog_runned_steps_count;
extern vector<FRAMObject<ProgramStep>*>   * prog_runned_steps;
extern FRAMObject<uint8_t>&                 prog_active_step;
extern FRAMObject<uint8_t>&                 prog_next_step;

extern FilterValue                        * filter_tempC_product;
extern uint8_t                              OptIn_state[8];
extern boolean                              Pressed_key_accept_for_prog;

extern ProgramStep                          prev_step;

struct __attribute__((packed)) ProgramControl
{
    ProgramAimEnum aim;
    TaskStateEnum state = TaskStateEnum::AWAIT;

    boolean is_runned = false;
    boolean on_pause_by_user = false;
    boolean on_pause_by_wd_wJacket_drain = false;
    boolean on_pause_by_wd_380v = false;
    S_DateTime started_at;
    S_DateTime last_iteration;
    uint32_t gone_ss;
    uint16_t limit_ss_max_await_on_pause;

    ProgramControl() :
    aim(ProgramAimEnum::NONE),
    state(TaskStateEnum::AWAIT),
    is_runned(false),
    started_at(S_DateTime()),
    last_iteration(S_DateTime()),
    gone_ss(0),
    limit_ss_max_await_on_pause(3600)
    {}

    int64_t seconds() {
        return millis() / 1000;
    }

    bool is_last_step(uint16_t index) {
        return index == prog_runned_steps_count.get() - 1;
    }

    void resume_task() {
        if (this->is_runned)
            on_pause_by_user = false;
    }

    void pause_task(bool permanent_pause = false)
    {
        if (this->is_runned)
        {
            if (!permanent_pause)
                on_pause_by_user = !on_pause_by_user;
            else
                on_pause_by_user = true;
        }
    }

    void end_task_by_user()
    {
        if (this->is_runned)
            this->state = TaskStateEnum::ERROR;
    }

    void pause_state_by_wd_wJacket_drain(boolean must_be_on_pause)
    {
        if (this->is_runned)
            on_pause_by_wd_wJacket_drain = must_be_on_pause;
    }

    void pause_state_by_wd_380v(boolean must_be_on_pause)
    {
        if (this->is_runned)
            on_pause_by_wd_380v = must_be_on_pause;
    }

    double get_prog_percentage();
    uint32_t sum_gone_ss();
    ProgramStep * get_prev_step();
    ProgramStep do_task(bool first_call_after_startup = false);
    ProgramStep start_task(ProgramAimEnum aim, uint16_t limit_ss_max_await_on_pause = 3600);
    
private:
    TaskStateEnum get_task_state() {
        return this->state;
    }
};

/* Prepeared templates cascade for TMPE/P and CHM  */
struct __attribute__((packed)) TMPEProgramTemplate
{
    ProgramStep step_pasteurising;
    ProgramStep step_cooling;
    ProgramStep step_heating;

    TMPEProgramTemplate() : TMPEProgramTemplate(
        ProgramStep(ProgramStepAimEnum::PASTEUR,    0, 0, 0, 0, 0, 0, 0),
        ProgramStep(ProgramStepAimEnum::CHILLING,   0, 0, 0, 0, 0, 0, 0),
        ProgramStep(ProgramStepAimEnum::HEATING,    0, 0, 0, 0, 0, 0, 0)
    )
    {}

    TMPEProgramTemplate(
        ProgramStep step_pasteurising,
        ProgramStep step_cooling,
        ProgramStep step_heating
    ) :
    step_pasteurising(step_pasteurising),
    step_cooling(step_cooling),
    step_heating(step_heating)
    {}

    ProgramStep * get_step(uint8_t index)
    {
        switch (index)
        {
        case 0: return &step_pasteurising; break;
        case 1: return &step_cooling; break;
        case 2: return &step_heating; break;
        default:
            return &step_pasteurising;
            break;
        }
    }
};

struct __attribute__((packed)) CHMProgramTemplate
{
    ProgramStep step_pasteurising;
    ProgramStep step_cooling;
    ProgramStep step_cutting;
    ProgramStep step_mixing;
    ProgramStep step_heating;
    ProgramStep step_drying;

    CHMProgramTemplate() : CHMProgramTemplate(
        ProgramStep(ProgramStepAimEnum::PASTEUR,  0, 0, 0, 0, 0, 0, 0),
        ProgramStep(ProgramStepAimEnum::CHILLING, 0, 0, 0, 0, 0, 0, 0),
        ProgramStep(ProgramStepAimEnum::CUTTING,  0, 0, 0, 0, 0, 0, 0),
        ProgramStep(ProgramStepAimEnum::MIXING,   0, 0, 0, 0, 0, 0, 0),
        ProgramStep(ProgramStepAimEnum::HEATING,  0, 0, 0, 0, 0, 0, 0),
        ProgramStep(ProgramStepAimEnum::DRYING,   0, 0, 0, 0, 0, 0, 0)
    )
    {}

    CHMProgramTemplate(
        ProgramStep step_pasteurising,
        ProgramStep step_cooling,
        ProgramStep step_cutting,
        ProgramStep step_mixing,
        ProgramStep step_heating,
        ProgramStep step_drying
    ) :
    step_pasteurising(step_pasteurising),
    step_cooling(step_cooling),
    step_cutting(step_cutting),
    step_mixing(step_mixing),
    step_heating(step_heating),
    step_drying(step_drying)
    {}

    ProgramStep * get_step(uint8_t index)
    {
        switch (index)
        {
        case 0: return &step_pasteurising; break;
        case 1: return &step_cooling; break;
        case 2: return &step_cutting; break;
        case 3: return &step_mixing; break;
        case 4: return &step_heating; break;
        case 5: return &step_drying; break;
        default:
            return &step_pasteurising;
            break;
        }
    }
};

static ProgramStep prog_null_step = ProgramStep();

#endif