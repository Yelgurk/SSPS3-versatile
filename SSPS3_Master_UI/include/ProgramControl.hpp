#pragma once

#ifndef ProgramControl_hpp
#define ProgramControl_hpp

#include <Arduino.h>
#include "../../SSPS3_Master_Domain/include/DateTime/S_DateTime.hpp"
#include "../include/UIControls/UITaskRoadmapList.hpp"

extern S_DateTime * dt_rt;

enum class ProgramAimEnum : uint8_t
{
    NONE,
    TMP_PASTEUR,
    TMP_HEAT,
    TMP_CHILL,
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
    CHM_TEMPL_6
};

enum class ProgramStepAimEnum : uint8_t
{
    WATER_JACKET,
    PASTEUR,
    CHILLING,
    CUTTING,
    MIXING,
    HEATING,
    DRYING
};

struct __attribute__((packed)) ProgramStep
{
    ProgramStepAimEnum aim;
    uint8_t fan;
    uint8_t tempC;
    uint32_t duration_ss;
    bool must_be_cooled;
    bool await_ok_button;
    bool step_is_turned_on;

    StepStateEnum state = StepStateEnum::AWAIT;
    uint32_t gone_ss = 0;

    ProgramStep() : ProgramStep(ProgramStepAimEnum::PASTEUR, 0, 0, 0, 0, 0, 0) {}

    ProgramStep(
        ProgramStepAimEnum aim,
        uint8_t fan,
        uint8_t tempC,
        uint32_t duration_ss,
        bool must_be_cooled,
        bool await_ok_button,
        bool step_is_turned_on
    ) :
    aim(aim),
    fan(fan),
    tempC(tempC),
    duration_ss(duration_ss),
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

static vector<ProgramStep> * steps;
static ProgramStep * current_step = nullptr;
static ProgramStep * next_step = nullptr;

struct __attribute__((packed)) ProgramControl
{
    ProgramAimEnum aim;
    TaskStateEnum state = TaskStateEnum::AWAIT;

    boolean is_runned = false;
    S_DateTime started_at;
    S_DateTime last_iteration;
    uint32_t gone_ss;

    ProgramControl() :
    aim(ProgramAimEnum::NONE),
    state(TaskStateEnum::AWAIT),
    is_runned(false),
    started_at(S_DateTime()),
    last_iteration(S_DateTime()),
    gone_ss(0)
    {}

    int64_t seconds() {
        return millis() / 1000;
    }

    bool is_last_step(uint16_t index) {
        return index == steps->size() - 1;
    }

    bool is_last_step(ProgramStep * step) {
        return &steps->at(steps->size() - 1) == step;
    }

    void resume_task()
    {
        if (get_task_state() == TaskStateEnum::PAUSE)
            set_task_state(TaskStateEnum::RUNNED);
    }

    void pause_task(bool permanent_pause = false)
    {
        if (!permanent_pause)
        {
            if (get_task_state() == TaskStateEnum::RUNNED)
                set_task_state(TaskStateEnum::PAUSE);
            else if (get_task_state() == TaskStateEnum::PAUSE)
                set_task_state(TaskStateEnum::RUNNED);
        }
        else
            set_task_state(TaskStateEnum::PAUSE);
    }

    void end_task()
    {
        set_task_state(TaskStateEnum::ERROR);
    }

    double get_prog_percentage();
    ProgramStep * start_task(ProgramAimEnum aim, vector<ProgramStep> * _steps);
    uint32_t sum_gone_ss();
    ProgramStep * do_task();
    
private:
    void set_task_state(TaskStateEnum task_new_state)
    {
        if (this->is_runned)
            this->state = task_new_state;
    }

    TaskStateEnum get_task_state() {
        return this->state;
    }
};

struct __attribute__((packed)) TMPEProgramTemplate
{
    ProgramStep step_pasteurising;
    ProgramStep step_cooling;
    ProgramStep step_heating;

    TMPEProgramTemplate() : TMPEProgramTemplate(
        ProgramStep(ProgramStepAimEnum::PASTEUR,    0, 0, 0, 0, 0, 0),
        ProgramStep(ProgramStepAimEnum::CHILLING,   0, 0, 0, 0, 0, 0),
        ProgramStep(ProgramStepAimEnum::HEATING,    0, 0, 0, 0, 0, 0)
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
        ProgramStep(ProgramStepAimEnum::PASTEUR,  0, 0, 0, 0, 0, 0),
        ProgramStep(ProgramStepAimEnum::CHILLING, 0, 0, 0, 0, 0, 0),
        ProgramStep(ProgramStepAimEnum::CUTTING,  0, 0, 0, 0, 0, 0),
        ProgramStep(ProgramStepAimEnum::MIXING,   0, 0, 0, 0, 0, 0),
        ProgramStep(ProgramStepAimEnum::HEATING,  0, 0, 0, 0, 0, 0),
        ProgramStep(ProgramStepAimEnum::DRYING,   0, 0, 0, 0, 0, 0)
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
};

#endif