#pragma once

#ifndef UIClock_hpp
#define UIClock_hpp

#include "../UIElement.hpp"

class UIClock : public UIElement
{
public:
    UIClock(UIScreen * parent, vector<KeyModel> keys_action):
        UIElement{parent, false, {UIAccess::All}, keys_action}
    {
        lv_obj_set_width(container, 100);
        lv_obj_set_height(container, 40);
        
        lv_obj_set_style_bg_color(container, lv_color_hex(0xFF9966), LV_PART_MAIN | LV_STATE_FOCUSED);
        
        lv_obj_t * time = lv_label_create(container);
        lv_obj_align(time, LV_ALIGN_TOP_MID, 0, 0);
        lv_label_set_text(time, "12:00:30");

        lv_obj_t * date = lv_label_create(container);
        lv_obj_align(date, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_label_set_text(date, "19.06.2024");

        childs.insert({"[time]", time});
        childs.insert({"[date]", date});
    }
};

#endif