#include "../include/ProgramControl.hpp"

double ProgramControl::get_prog_percentage()
{
    int32_t aim_ss = 0,
            done_ss = 0;

    for (auto step : *steps)
    {
        aim_ss += step.duration;
        done_ss += (step.gone_ss > step.duration ? step.duration : step.gone_ss);
    }

    return aim_ss <= 0 ? 0 : 100.0 / (double)aim_ss * done_ss;
}

ProgramStep * ProgramControl::start_task(string name, vector<ProgramStep> * steps)
{
    if (!is_active)
    {
        this->name = name;
        this->steps = steps;

        last_iteration_ss = started_at_ss = seconds();
        current_step = next_step = nullptr;

        gone_ss = 0;
        state = TaskStateEnum::RUNNED;
        is_active = true;
    }

    return do_task();
}

ProgramStep * ProgramControl::do_task()
{
    uint32_t curr_ss = seconds();
    int32_t ss_from_last_iteration = curr_ss - last_iteration_ss;
    last_iteration_ss = curr_ss;

    if (is_active)
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
                    is_active = false;
                }
            }

            gone_ss = curr_ss - started_at_ss;
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
            is_active = false;
            /*
            return дефолт экземпляра ProgramStep у которого стоит всё по 0, т.к. управление будет согласно
            значениям сугубо по возвращенному в ProgramStep
            */
        }
    }
    
    return current_step;
}