#pragma once
#ifndef EXT_MEM_TASK_CLUSTER_H
#define EXT_MEM_TASK_CLUSTER_H

#ifdef DEV_SSPS3_RUN_ON_PLC
    #include "./container_task_page.h"
#else
    #include "container_task_page.h"
#endif

class ContainerTaskCluster
{
private:
    std::list<ContainerTaskPage>::iterator l_front;
    std::list<ContainerTaskPage> pages;
    ContainerTaskPage* _selected_page;

public:
    ContainerTaskCluster() {}

    ContainerTaskPage* create_page(XVar<std::string>& page_name)
    {
        pages.push_back(ContainerTaskPage(page_name));
        return &*(_selected_page = &pages.back());
    }

    ContainerTaskCluster* select_page(unsigned char index)
    {
        std::advance(
            l_front = pages.begin(),
            index >= pages.size() ? pages.size() - 1 : index
        );

        _selected_page = &(*l_front);

        select_instruction(0);
        return this;
    }

    //ContainerTaskCluster* select_instruction(unsigned char index)
    //{
    //    _selected_page->select_instruction(index);
    //    return this;
    //}

    ContainerTaskInstruction* select_instruction(unsigned char index)
    {
        _selected_page->select_instruction(index);
        return get_instruction();
    }

    ContainerTaskPage* get_page()
    {
        return _selected_page;
    }

    ContainerTaskInstruction* get_instruction()
    {
        return _selected_page->get_instruction();
    }
};

#endif // !MEM_EXT_PAGE_TEMPLATE_H
