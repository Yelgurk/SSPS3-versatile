#pragma once
#ifndef EXT_MEM_TASK_CLUSTER_H
#define EXT_MEM_TASK_CLUSTER_H

#ifdef DEV_SSPS3_RUN_ON_PLC
    #include "./ext_mem_task_page.h"
#else
    #include "ext_mem_task_page.h"
#endif

class ExtMemTaskCluster
{
private:
    //std::vector<ExtMemTaskPage> pages;

    std::list<ExtMemTaskPage>::iterator l_front;
    std::list<ExtMemTaskPage> pages;
    ExtMemTaskPage* _selected;

public:
    ExtMemTaskCluster() {}

    ExtMemTaskPage* create_page(XVar<std::string>& page_name)
    {
        pages.push_back(ExtMemTaskPage(page_name));
        return &*(_selected = &pages.back());
    }

    ExtMemTaskCluster* select_page(unsigned char index)
    {
        //_selected = &pages.at(index >= pages.size() ? pages.size() - 1 : index); | for case, when pages var is vector, not list

        std::advance(
            l_front = pages.begin(),
            index >= pages.size() ? pages.size() - 1 : index
        );

        _selected = &(*l_front);

        select_group(0);
        return this;
    }

    ExtMemTaskCluster* select_group(unsigned char index)
    {
        _selected->select_group(index);
        return this;
    }

    ExtMemTaskPage* get_page()
    {
        return _selected;
    }

    ExtMemTaskGroup* get_group()
    {
        return _selected->get_group();
    }
};

#endif // !MEM_EXT_PAGE_TEMPLATE_H
