#pragma once
#ifndef EXT_MEM_TASK_PAGE_H
#define EXT_MEM_TASK_PAGE_H

#ifdef DEV_SSPS3_RUN_ON_PLC
    #include <string>
    #include <list>
    #include "./ext_mem_task_collection.h"
#else
    #include <string>
    #include <vector>
    #include "ext_mem_task_collection.h"
#endif

class ExtMemTaskPage
{
private:
    //std::vector<ExtMemTaskGroup> stages;

    std::list<ExtMemTaskGroup>::iterator l_front;
    std::list<ExtMemTaskGroup> stages;
    ExtMemTaskGroup* _selected;

    XVar<std::string>& page_name;

public:
    ExtMemTaskPage(XVar<std::string>& page_name) :
        page_name(page_name),
        _selected(nullptr)
    {}

    ExtMemTaskPage* push(
        XVar<std::string>& name,
        XVar<unsigned char>& rpm,
        XVar<unsigned char>& temperature,
        XVar<unsigned short>& duration_ss,
        XVar<bool>& is_active_cooling,
        XVar<bool>& is_await_user_ok,
        XVar<bool>& is_stage_turned_on)
    {
        stages.push_back(ExtMemTaskGroup(name, rpm, temperature, duration_ss, is_active_cooling, is_await_user_ok, is_stage_turned_on));
        _selected = &stages.back();

        return this;
    }

    ExtMemTaskPage* select_group(unsigned char index)
    {
        //_selected = &stages.at(index >= stages.size() ? stages.size() - 1 : index); | the same like in ext_mem_task_cluster.h

        std::advance(
            l_front = stages.begin(),
            index >= stages.size() ? stages.size() - 1 : index
        );

        _selected = &(*l_front);

        _selected->refresh_ui();
        page_name.load_value_into_ui();
        return this;
    }

    ExtMemTaskGroup* get_group()
    {
        return _selected;
    }
};

#endif // ! MEM_EXT_STAGE_TEMPLATE_H
