#pragma once

#ifndef ProgramControl_hpp
#define ProgramControl_hpp

#include <Arduino.h>
#include "../../SSPS3_Master_Domain/include/DateTime/S_DateTime.hpp"
#include "../include/UIControls/UITaskRoadmapList.hpp"

enum class ProgramStepAimEnum : uint8_t
{
    PASTEUR,
    CHILLING,
    CUTTING,
    MIXING,
    HEATING,
    DRYING
};

enum class ProgramAimEnum : uint8_t
{
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

struct __attribute__((packed)) ProgramStep
{
    ProgramStepAimEnum aim;
    uint8_t fan;
    uint8_t tempC;
    uint8_t duration_ss;
    bool step_is_turned_on;
    bool await_ok_button;

    StepStateEnum state = StepStateEnum::AWAIT;
    uint32_t gone_ss = 0;

    ProgramStep(
        ProgramStepAimEnum aim,
        uint8_t fan,
        uint8_t tempC,
        uint32_t duration_ss,
        bool step_is_turned_on,
        bool await_ok_button
    ) :
    aim(aim),
    fan(fan),
    tempC(tempC),
    duration_ss(duration_ss),
    step_is_turned_on(step_is_turned_on),
    await_ok_button(await_ok_button)
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

    boolean is_active = false;
    S_DateTime started_at;
    S_DateTime last_iteration;
    uint32_t gone_ss;

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

    void pause_task()
    {
        if (get_task_state() == TaskStateEnum::RUNNED)
            set_task_state(TaskStateEnum::PAUSE);
        else if (get_task_state() == TaskStateEnum::PAUSE)
            set_task_state(TaskStateEnum::RUNNED);
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
        if (this->is_active)
            this->state = task_new_state;
    }

    TaskStateEnum get_task_state() {
        return this->state;
    }
};

#endif