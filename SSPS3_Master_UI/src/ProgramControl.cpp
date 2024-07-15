#include "../include/ProgramControl.hpp"

double ProgramControl::get_prog_percentage()
{
    int32_t aim_ss = 0,
            done_ss = 0;

    for (auto _steps : *steps)
    {
        aim_ss += _steps.duration_ss;
        done_ss += (_steps.gone_ss > _steps.duration_ss ? _steps.duration_ss : _steps.gone_ss);
    }

    return aim_ss <= 0 ? 0 : 100.0 / (double)aim_ss * done_ss;
}

ProgramStep * ProgramControl::start_task(ProgramAimEnum aim, vector<ProgramStep> * _steps)
{
    if (!is_runned)
    {
        this->aim = aim;
        steps = _steps;

        dt_rt->get_rt();
        
        started_at.set_date(*dt_rt->get_date());
        started_at.set_time(*dt_rt->get_time());

        last_iteration.set_date(*dt_rt->get_date());
        last_iteration.set_time(*dt_rt->get_time());

        current_step = next_step = nullptr;

        state = TaskStateEnum::RUNNED;
        is_runned = true;
    }

    return do_task();
}

uint32_t ProgramControl::sum_gone_ss()
{
    static uint32_t total_gone_ss = 0;
    total_gone_ss = 0;

    for (const auto& step : *steps)
        total_gone_ss += step.gone_ss;

    return total_gone_ss;
}

ProgramStep * ProgramControl::do_task()
{
    dt_rt->get_rt();

    int32_t ss_from_last_iteration = dt_rt->difference_in_seconds(last_iteration);
    last_iteration.set_date(*dt_rt->get_date());
    last_iteration.set_time(*dt_rt->get_time());

    if (is_runned)
    {
        for (int i = 0; i < steps->size(); i++)
        {
            if ( i == 0 && steps->at(i).state == StepStateEnum::AWAIT)
                steps->at(i).state = StepStateEnum::RUNNED;

            if (i < (int)(steps->size() - 1) && steps->at(i + 1).state == StepStateEnum::AWAIT)
            {
                current_step = &steps->at(i);
                next_step = &steps->at(i + 1);
                break;
            }

            if (i >= steps->size() - 1)
            {
                current_step = &steps->at(steps->size() - 1);
                next_step = &steps->at(steps->size() - 1);
                break;
            }
        }
        if (state == TaskStateEnum::RUNNED)
        {
            current_step->iteration_ss(ss_from_last_iteration);
            current_step->state = StepStateEnum::RUNNED;

            if (current_step->is_time_out())
            {
                current_step->state = StepStateEnum::DONE;
                if (!is_last_step(current_step))
                    next_step->state = StepStateEnum::RUNNED;
                else
                {
                    state = TaskStateEnum::DONE;
                    is_runned = false;
                }
            }

            gone_ss = sum_gone_ss();
        }
        else if (state == TaskStateEnum::PAUSE)
        {
            current_step->state = StepStateEnum::PAUSE;
            /*
            return дефолт экземпляра ProgramStep у которого стоит всё по 0, т.к. управление будет согласно
            значениям сугубо по возвращенному в ProgramStep
            */
        }
        else if (state == TaskStateEnum::ERROR)
        {
            current_step->state = StepStateEnum::ERROR;
            is_runned = false;
            /*
            return дефолт экземпляра ProgramStep у которого стоит всё по 0, т.к. управление будет согласно
            значениям сугубо по возвращенному в ProgramStep
            */
        }
    }
    
    return current_step;
}