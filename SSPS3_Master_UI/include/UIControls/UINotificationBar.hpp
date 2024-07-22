#ifndef UINotificationBar_hpp
#define UINotificationBar_hpp

#include "../UIElement.hpp"

enum class SystemNotification
{
    NONE = 0,
_OK_BEGIN = 1,
    OK_TASK_DONE,
    OK_BLOWING_DONE,
_OK_END = 20,
_INFO_BEGIN = 21,
    INFO_TURN_OFF_380V_FIRST,
    INFO_TURN_ON_380V_FIRST,
    INFO_BLOWING_RESET_2_SS_AWAIT,
    INFO_BLOWING_CANCELED_BY_USER,
    INFO_TASK_AWAIT_PROBLEM_SOLVING,
    INFO_TASK_RESUME_AFTER_PROBLEM_SOLVING,
    INFO_TASK_RESUME_AFTER_PLC_REBOOT,
    INFO_TASK_CANCELED_BY_USER,
_INFO_END = 40,
_WARNING_BEGIN = 41,
    WARNING_WATER_JACKET_NO_WATER,
    WARNING_380V_NO_POWER,
_WARNING_END = 60,
_ERROR_BEGIN = 61,
    ERROR_3_PHASE_MOTOR_IS_BROKEN,
    ERROR_TEMP_C_SENSOR_BROKEN,
    ERROR_TASK_ENDED_30_MIN_SOLVING_AWAIT,
_ERROR_END = 80
};

struct SystemNotificationContainer
{
    SystemNotification info = SystemNotification::NONE;
    boolean must_be_closed = false;

    SystemNotificationContainer() {}
    SystemNotificationContainer(SystemNotification info) : info(info) {}
};

class UINotificationBar;

static UINotificationBar* instance = nullptr;
static bool animation_in_action = false;

class UINotificationBar : public UIElement
{
private:
    vector<SystemNotificationContainer> notifications;
    uint16_t index = 0;

    lv_anim_t a;

    static void anim_callback(void* obj, int32_t  v) {
        lv_obj_set_y((lv_obj_t*)obj, v);
    }

    static void anim_ready(lv_anim_t* anim)
    {
        animation_in_action = false;
        instance->show_next();
    }

    void show_next()
    {
        if (!notifications.empty())
        {  
            if (index >= notifications.size())
                index = 0;

            animation_in_action = true;

            static lv_obj_t * img = get_container_content("[notify_img]");
            static lv_obj_t * text = get_container_content("[notify_text]");

            if (notifications.at(index).info < SystemNotification::_OK_END)
                lv_image_set_src(img, &img_checkmark);
            else if (notifications.at(index).info < SystemNotification::_INFO_END)
                lv_image_set_src(img, &img_info);
            else if (notifications.at(index).info < SystemNotification::_WARNING_END)
                lv_image_set_src(img, &img_warning);
            else if (notifications.at(index).info < SystemNotification::_ERROR_END)
                lv_image_set_src(img, &img_error);

            switch (notifications.at(index).info)
            {
            case SystemNotification::OK_TASK_DONE:                              lv_label_set_text(text, "Программа полностью выполнена!"); break;
            case SystemNotification::OK_BLOWING_DONE:                           lv_label_set_text(text, "Раздача завершена!"); break;
            case SystemNotification::INFO_TURN_OFF_380V_FIRST:                  lv_label_set_text(text, "Сначала ВЫКЛючите 380В"); break;
            case SystemNotification::INFO_TURN_ON_380V_FIRST:                   lv_label_set_text(text, "Сначала ВКЛючите 380В"); break;
            case SystemNotification::INFO_BLOWING_RESET_2_SS_AWAIT:             lv_label_set_text(text, "Сброс раздачи, 2 секунды ожидания"); break;
            case SystemNotification::INFO_BLOWING_CANCELED_BY_USER:             lv_label_set_text(text, "Сброс задачи пользователем"); break;
            case SystemNotification::INFO_TASK_AWAIT_PROBLEM_SOLVING:           lv_label_set_text(text, "Ожидание разрешения проблем пользователем"); break;
            case SystemNotification::INFO_TASK_RESUME_AFTER_PROBLEM_SOLVING:    lv_label_set_text(text, "Программа продолжена после разрешения проблем"); break;
            case SystemNotification::INFO_TASK_RESUME_AFTER_PLC_REBOOT:         lv_label_set_text(text, "Программа продолжена после перезагрузки ПЛК"); break;
            case SystemNotification::INFO_TASK_CANCELED_BY_USER:                lv_label_set_text(text, "Программа преравана пользователем по кнопке"); break;
            case SystemNotification::WARNING_WATER_JACKET_NO_WATER:             lv_label_set_text(text, "ВНИМАНИЕ! Нет воды в рубашке!"); break;
            case SystemNotification::WARNING_380V_NO_POWER:                     lv_label_set_text(text, "ВНИМАНИЕ! Нет сети 380В!"); break;
            case SystemNotification::ERROR_3_PHASE_MOTOR_IS_BROKEN:             lv_label_set_text(text, "ОШИБКА! Авария мешалки!"); break;
            case SystemNotification::ERROR_TEMP_C_SENSOR_BROKEN:                lv_label_set_text(text, "ОШИБКА! Температурный дачтик вышел из строя!"); break;
            case SystemNotification::ERROR_TASK_ENDED_30_MIN_SOLVING_AWAIT:     lv_label_set_text(text, "ОШИБКА! Принудительное завершение программы: 30 минут простоя без решения проблем!"); break;
            
            default:
                lv_label_set_text(text, "Тут будут уведомления!");
                break;
            }

            if ((notifications[index].must_be_closed) ||
                (notifications[index].info >= SystemNotification::_WARNING_BEGIN && notifications[index].info <= SystemNotification::_WARNING_END))
                notifications.erase(notifications.begin() + index);
            else
                ++index;
   
            lv_anim_start(&a);
        }
    }

public:
    UINotificationBar(
        lv_obj_t * lv_screen
    ) : UIElement {
        { EquipmentType::All },
        {
            KeyModel(KeyMap::L_STACK_4, [this]() { this->display(true); }),
            KeyModel(KeyMap::L_STACK_3, [this]() { this->display(true); }),
            KeyModel(KeyMap::L_STACK_2, [this]() { this->display(true); }),
            KeyModel(KeyMap::L_STACK_1, [this]() { this->display(true); }),
            KeyModel(KeyMap::LEFT_TOP,  [this]() { this->display(true); }),
            KeyModel(KeyMap::LEFT_BOT,  [this]() { this->display(true); }),
            KeyModel(KeyMap::LEFT,      [this]() { this->display(true); }),
            KeyModel(KeyMap::TOP,       [this]() { this->display(true); }),
            KeyModel(KeyMap::RIGHT,     [this]() { this->display(true); }),
            KeyModel(KeyMap::BOTTOM,    [this]() { this->display(true); }),
            KeyModel(KeyMap::RIGHT_TOP, [this]() { this->display(true); }),
            KeyModel(KeyMap::RIGHT_BOT, [this]() { this->display(true); }),
            KeyModel(KeyMap::R_STACK_4, [this]() { this->display(true); }),
            KeyModel(KeyMap::R_STACK_3, [this]() { this->display(true); }),
            KeyModel(KeyMap::R_STACK_2, [this]() { this->display(true); }),
            KeyModel(KeyMap::R_STACK_1, [this]() { this->display(true); })
        },
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
        lv_obj_set_height(get_container(), 50);
        lv_obj_align(get_container(), LV_ALIGN_TOP_MID, 0, -60);
        lv_obj_remove_flag(get_container(), LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_bg_color(get_container(), COLOR_WHITE, 0);
        lv_obj_set_style_bg_color(get_container(), COLOR_WHITE, LV_PART_MAIN | LV_STATE_USER_1);
        lv_obj_set_style_bg_color(get_container(), COLOR_YELLOW_SMOKE, LV_PART_MAIN | LV_STATE_USER_2);
        lv_obj_set_style_bg_color(get_container(), COLOR_PINK, LV_PART_MAIN | LV_STATE_USER_3);

        lv_obj_t * notify_icon = lv_image_create(get_container());
        lv_obj_align(notify_icon, LV_ALIGN_LEFT_MID, 10, 0);
        lv_image_set_src(notify_icon, &img_checkmark);

        lv_obj_t * notify_text = lv_label_create(get_container());
        lv_obj_align(notify_text, LV_ALIGN_LEFT_MID, 60, 0);
        lv_label_set_text(notify_text, "Здесь будут уведомления");
        lv_obj_set_style_text_font(notify_text, &OpenSans_bold_24px, LV_PART_MAIN | LV_STATE_DEFAULT);
        //lv_label_set_long_mode(notify_text, LV_LABEL_LONG_SCROLL_CIRCULAR);
        //lv_obj_set_width(notify_text, LV_SIZE_CONTENT);
        //lv_obj_set_style_max_width(notify_text, width_selector - 20, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_anim_init(&a);
        lv_anim_set_var(&a, get_container());
        lv_anim_set_values(&a, -60, 0);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
        lv_anim_set_time(&a, 600);
        lv_anim_set_playback_delay(&a, 4000);
        lv_anim_set_playback_time(&a, 300);
        lv_anim_set_exec_cb(&a, anim_callback);
        lv_anim_set_completed_cb(&a, anim_ready);

        remember_child_element("[notify_img]", notify_icon);
        remember_child_element("[notify_text]", notify_text);

        instance = this;
    }

    void push_info(SystemNotification type)
    {
        auto it = std::find_if(
            notifications.begin(),
            notifications.end(),
            [=](const SystemNotificationContainer& container) { return container.info == type; }
        );

        if (it == notifications.end())
            notifications.push_back(SystemNotificationContainer(type));

        if (!animation_in_action)
            display();
    }

    void display(bool button_pressed = false)
    {
        if (button_pressed)
            for (auto& container : notifications)
                if ((container.info >= SystemNotification::_OK_BEGIN && container.info <= SystemNotification::_INFO_END) ||
                    (container.info >= SystemNotification::_ERROR_BEGIN && container.info <= SystemNotification::_ERROR_END))
                    container.must_be_closed = true;

        if (!animation_in_action)
            show_next();
    }
};

#endif