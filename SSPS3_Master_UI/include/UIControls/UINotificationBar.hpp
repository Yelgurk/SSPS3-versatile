#ifndef UINotificationBar_hpp
#define UINotificationBar_hpp

#include "../UIElement.hpp"

class UINotificationBar : public UIElement
{
public:
    enum class NotifyType {
        OK,
        ERROR,
        WARNING,
        INFO
    };

private:
    lv_obj_t* parent;
    lv_obj_t* bar;
    std::vector<NotifyType> notifications;
    std::vector<NotifyType> critical_notifications;

    static void anim_callback(void* anim, int32_t v);
    static void anim_ready(lv_anim_t* anim);

    static UINotificationBar* instance;

    void show_next()
    {
        if (!critical_notifications.empty())
        {
            NotifyType type = critical_notifications.front();
            critical_notifications.erase(critical_notifications.begin());

            lv_color_t color;
            const char* text;

            switch (type) {
                case NotifyType::OK: color = COLOR_GREEN; text = "OK"; break;
                case NotifyType::ERROR: color = COLOR_RED; text = "ERROR"; break;
                case NotifyType::WARNING: color = COLOR_YELLOW; text = "WARNING"; break;
                case NotifyType::INFO: color = COLOR_BLUE; text = "INFO"; break;
            }

            lv_obj_set_style_bg_color(bar, color, LV_PART_MAIN);
            lv_label_set_text(lv_label_create(bar), text);

            lv_obj_clear_flag(bar, LV_OBJ_FLAG_HIDDEN);

            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, bar);
            lv_anim_set_values(&a, -50, 0);
            lv_anim_set_time(&a, 500);
            lv_anim_set_exec_cb(&a, anim_callback);
            lv_anim_set_completed_cb(&a, anim_ready);
            lv_anim_start(&a);
        }
        else if (!notifications.empty())
        {
            NotifyType type = notifications.front();
            if (type == NotifyType::INFO)
            {
                notifications.erase(notifications.begin());
            }
            else
            {
                critical_notifications.push_back(type);
                notifications.erase(notifications.begin());
            }
            show_next();
        }
        else
        {
            lv_obj_add_flag(bar, LV_OBJ_FLAG_HIDDEN);
        }
    }

public:
    UINotificationBar(
        lv_obj_t * lv_screen
    ) : UIElement {
        { EquipmentType::All },
        { },
        false,
        false,
        true,
        PlaceControlIn::Screen,
        lv_screen,
        nullptr,
        { StyleActivator::Shadow, StyleActivator::Rectangle, StyleActivator::Unscrollable }
    }
    {
        lv_obj_set_width(get_container(), 480);
        lv_obj_set_height(get_container(), 40);
        lv_obj_align(get_container(), LV_ALIGN_TOP_MID, 0, -50);
        lv_obj_add_flag(bar, LV_OBJ_FLAG_HIDDEN);
        instance = this;
    }

    void push_info(NotifyType type)
    {
        notifications.push_back(type);
    }

    void display(bool button_pressed = false)
    {
        if (button_pressed)
        {
            critical_notifications.insert(critical_notifications.end(), notifications.begin(), notifications.end());
            notifications.clear();
        }
        show_next();
    }
};

UINotificationBar* UINotificationBar::instance = nullptr;

void UINotificationBar::anim_callback(void* obj, int32_t  v)
{
    lv_obj_set_y((lv_obj_t*)obj, v);
}

void UINotificationBar::anim_ready(lv_anim_t* anim)
{
    if (lv_obj_get_y((lv_obj_t*)anim->var) == 0) {
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, anim->var);
        lv_anim_set_values(&a, 0, -50);
        lv_anim_set_time(&a, 500);
        lv_anim_set_delay(&a, 2000);
        lv_anim_set_exec_cb(&a, anim_callback);
        lv_anim_set_ready_cb(&a, [](lv_anim_t* anim) {
            instance->show_next();
        });
        lv_anim_start(&a);
    }
}

#endif