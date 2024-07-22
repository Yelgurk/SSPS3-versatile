#include "../include/ProgramControl.hpp"

double ProgramControl::get_prog_percentage()
{
    int32_t aim_ss = 0,
            done_ss = 0;

    for (uint8_t i = 0; i < prog_runned_steps_count.get(); i++)
    {
        ProgramStep * _step = prog_runned_steps->at(i)->ptr();
        aim_ss += _step->duration_ss;
        done_ss += min(_step->gone_ss, _step->duration_ss);
    }

    return aim_ss <= 0 ? 0 : 100.0 / (double)aim_ss * done_ss;
}

ProgramStep ProgramControl:: start_task(ProgramAimEnum aim, uint16_t limit_ss_max_await_on_pause)
{
    if (!is_runned)
    {
        this->aim = aim;
        this->limit_ss_max_await_on_pause = limit_ss_max_await_on_pause;

        started_at.set_date(*dt_rt->get_date());
        started_at.set_time(*dt_rt->get_time());

        last_iteration.set_date(*dt_rt->get_date());
        last_iteration.set_time(*dt_rt->get_time());

        prog_active_step.set(0);
        prog_next_step.set(0);

        state = TaskStateEnum::RUNNED;
        is_runned = true;
    }

    return do_task();
}

uint32_t ProgramControl::sum_gone_ss()
{
    static uint32_t total_gone_ss = 0;
    total_gone_ss = 0;

    for (uint8_t i = 0; i < prog_runned_steps_count.get(); i++)
    {
        ProgramStep * _step = prog_runned_steps->at(i)->ptr();
        total_gone_ss += _step->gone_ss;
    }

    return total_gone_ss;
}

ProgramStep ProgramControl::do_task(bool first_call_after_startup)
{
    if (!this->is_runned)
    {
        switch (this->state)
        {
        case TaskStateEnum::AWAIT:  return prog_null_step; break;
        case TaskStateEnum::DONE:   return prog_null_step; break;
        case TaskStateEnum::ERROR:  return prog_null_step; break;
        default:
            break;
        }
    }

    static int32_t ss_from_last_iteration = 0;
    static ProgramStep _step_curr;
    static ProgramStep _step_next;
    static ProgramStep _response;
    static boolean _was_on_pause;
    static boolean _go_next_step;

    _was_on_pause = this->state == TaskStateEnum::PAUSE;
    _go_next_step = false;

    if (this->state == TaskStateEnum::RUNNED || this->state == TaskStateEnum::PAUSE)
        this->state =
            !on_pause_by_user && !on_pause_by_wd_wJacket_drain && !on_pause_by_wd_380v
            ? TaskStateEnum::RUNNED
            : TaskStateEnum::PAUSE;

    if (_was_on_pause && this->state == TaskStateEnum::RUNNED)
    {
        last_iteration.set_date(*dt_rt->get_date());
        last_iteration.set_time(*dt_rt->get_time());
    }

    if (is_runned)
    {
        for (int i = 0; i < prog_runned_steps_count.get(); i++)
        {
            ProgramStep _step_obs = prog_runned_steps->at(i)->get();
            if (i == 0 && _step_obs.state == StepStateEnum::AWAIT)
            {
                _step_obs.state = StepStateEnum::RUNNED;
                prog_runned_steps->at(i)->set(_step_obs);
            }

            if (i < prog_runned_steps_count.get() - 1)
            {
                ProgramStep _step_next = prog_runned_steps->at(i + 1)->get();

                if (_step_next.state == StepStateEnum::AWAIT)
                {
                    prog_active_step.set(i);
                    prog_next_step.set(i + 1);
                    break;
                }
            }

            if (i >= prog_runned_steps_count.get() - 1)
            {
                prog_active_step.set(i);
                prog_next_step.set(i);
                break;
            }
        }

        _step_curr = prog_runned_steps->at(prog_active_step.get())->get();
        _step_next = prog_runned_steps->at(prog_next_step.get())->get();
        _response = _step_curr;

        if (state == TaskStateEnum::RUNNED)
        {
            ss_from_last_iteration = dt_rt->difference_in_seconds(last_iteration);
            last_iteration.set_date(*dt_rt->get_date());
            last_iteration.set_time(*dt_rt->get_time());

            ss_from_last_iteration = first_call_after_startup ? 0 : ss_from_last_iteration;

            this->state = TaskStateEnum::RUNNED;
            _step_curr.iteration_ss(ss_from_last_iteration);
            _step_curr.state = StepStateEnum::RUNNED;

            if (_step_curr.aim == ProgramStepAimEnum::WATER_JACKET)
                _go_next_step = OptIn_state[DIN_WJACKET_SENS];
            else if (_step_curr.await_ok_button)
                _go_next_step = Pressed_key_accept_for_prog;
            else if (_step_curr.untill_condition_met)
                _go_next_step = IsInBounds<int16_t>(filter_tempC_product->get_physical_value(), _step_curr.tempC - 1, _step_curr.tempC + 1);
            else if (_step_curr.is_time_out())
                _go_next_step = true;

            if (_go_next_step)
            {
                _step_curr.state = StepStateEnum::DONE;
                if (!is_last_step(prog_active_step.get()))
                    _step_next.state = StepStateEnum::RUNNED;
                else
                {
                    this->state = TaskStateEnum::DONE;
                    is_runned = false;
                }
            }

            gone_ss = sum_gone_ss();

            prog_runned_steps->at(prog_active_step.get())->set(_step_curr);
            if (!is_last_step(prog_active_step.get()))
                prog_runned_steps->at(prog_next_step.get())->set(_step_next);
        }
        else if (state == TaskStateEnum::PAUSE)
        {
            _step_curr.state = StepStateEnum::PAUSE;

            if (dt_rt->difference_in_seconds(last_iteration) >= limit_ss_max_await_on_pause)
            {
                this->state = TaskStateEnum::ERROR;
                _step_curr.state = StepStateEnum::ERROR;
                this->is_runned = false;
            }

            prog_runned_steps->at(prog_active_step.get())->set(_step_curr);
            _response = prog_null_step;
        }
        else if (state == TaskStateEnum::ERROR)
        {
            _step_curr.state = StepStateEnum::ERROR;
            this->is_runned = false;

            prog_runned_steps->at(prog_active_step.get())->set(_step_curr);
            _response = prog_null_step;
        }

        Pressed_key_accept_for_prog = false;
    }
    else
        return prog_null_step;
    
    return _response;
}