#include <Arduino.h>
#include <map>
#include <vector>
#include <functional>

#ifndef RtTaskManager_hpp
#define RtTaskManager_hpp

using Task = std::function<void()>;

struct TaskInfo
{
    Task task;
    int64_t interval;
    int64_t last_run;
    bool active;

    TaskInfo() : interval(0), last_run(0), active(false) {}

    TaskInfo(Task task, int64_t interval, int64_t last_run, bool active)
    : task(task), interval(interval), last_run(last_run), active(active)
    {}
};

class RtTaskManager
{
private:
    std::map<string, TaskInfo> tasks;

public:
    void add_task(const string& name, Task task, int64_t interval) {
        tasks.insert({name, TaskInfo(task, interval, millis(), true)});
    }

    std::vector<string> get_tasks() const
    {
        std::vector<string> task_names;
        for (const auto& task : tasks)
        {
            task_names.push_back(task.first);
        }
        return task_names;
    }

    void stop_task(const string& name)
    {
        if (tasks.find(name) != tasks.end())
        {
            tasks[name].active = false;
        }
    }

    void execute_task(const string& name)
    {
        if (tasks.find(name) != tasks.end())
        {
            tasks[name].task();
            tasks[name].last_run = millis();
        }
    }

    void resume_task(const string& name)
    {
        if (tasks.find(name) != tasks.end())
        {
            tasks[name].active = true;
            tasks[name].last_run = millis();
        }
    }

    void remove_task(const string& name)
    {
        tasks.erase(name);
    }

    void run()
    {
        int32_t current_time = millis();
        for (auto& task : tasks)
        {
            if (task.second.active && (current_time - task.second.last_run >= task.second.interval))
            {
                task.second.task();
                task.second.last_run = current_time;
            }
        }
    }
};

#endif