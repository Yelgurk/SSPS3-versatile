#pragma once

#ifndef ProgramControl_hpp
#define ProgramControl_hpp

#include <Arduino.h>
#include "../include/UIControls/UITaskRoadmapList.hpp"

struct ProgramStep
{
    string name;
    uint8_t fan,
            tempC;
    uint32_t duration;
    uint32_t gone_ss = 0;
    StepStateEnum state = StepStateEnum::AWAIT;
    bool await_ok_button = false;

    ProgramStep(string name, uint8_t fan, uint8_t tempC, uint32_t duration)
    : name(name), fan(fan), tempC(tempC), duration(duration)
    {}

    bool is_time_out() {
        return gone_ss >= duration;
    }

    void iteration_ss(uint32_t ss) {
        gone_ss += ss;
    }

    int32_t time_left_ss() {
        return duration - gone_ss;
    }
};

struct ProgramControl
{
    string name;
    boolean is_active = false;
    TaskStateEnum state = TaskStateEnum::AWAIT;

    int64_t started_at_ss;
    int64_t last_iteration_ss;
    int64_t gone_ss;

    vector<ProgramStep> * steps;
    ProgramStep * current_step = nullptr;
    ProgramStep * next_step = nullptr;

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
    ProgramStep * start_task(string name, vector<ProgramStep> * steps);
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