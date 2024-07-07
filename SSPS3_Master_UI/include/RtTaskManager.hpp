#include <Arduino.h>
#include <map>
#include <vector>
#include <functional>

#ifndef RtTaskManager_hpp
#define RtTaskManager_hpp

class RtTaskManager
{
public:
    using Task = std::function<void()>;

    void add_task(const String& name, Task task, int32_t interval)
    {
        tasks[name] = {task, interval, millis(), true};
    }

    std::vector<String> get_tasks() const
    {
        std::vector<String> task_names;
        for (const auto& task : tasks)
        {
            task_names.push_back(task.first);
        }
        return task_names;
    }

    void stop_task(const String& name)
    {
        if (tasks.find(name) != tasks.end())
        {
            tasks[name].active = false;
        }
    }

    void resume_task(const String& name)
    {
        if (tasks.find(name) != tasks.end())
        {
            tasks[name].active = true;
            tasks[name].last_run = millis();
        }
    }

    void remove_task(const String& name)
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

private:
    struct TaskInfo
    {
        Task task;
        int32_t interval;
        int32_t last_run;
        bool active;
    };

    std::map<String, TaskInfo> tasks;
};

#endif