#ifndef RectangleProgressBar_hpp
#define RectangleProgressBar_hpp

#include <lvgl.h>

class RectangleProgressBar {
public:
    RectangleProgressBar(lv_obj_t* parent, int width, int height, int radius)
        : width(width), height(height), radius(radius) {
        // Создаем объект
        bar = lv_obj_create(parent);
        lv_obj_set_size(bar, width, height); // Устанавливаем размер
        lv_obj_center(bar);
        lv_obj_add_event_cb(bar, draw_event_cb, LV_EVENT_DRAW_MAIN, this);

        // Устанавливаем начальный прогресс
        set_value(0);
    }

    // Метод для установки значения прогресса
    void set_value(int value) {
        if (value < 0) value = 0;
        if (value > 10000) value = 10000;
        this->value = value;
        lv_obj_invalidate(bar); // Перерисовываем объект
    }

private:
    lv_obj_t* bar;
    int value;
    int width;
    int height;
    int radius;

    // Функция обратного вызова для рисования
    static void draw_event_cb(lv_event_t* e) {
        RectangleProgressBar* self = reinterpret_cast<RectangleProgressBar*>(lv_event_get_user_data(e));
        lv_obj_t* obj = (lv_obj_t*)lv_event_get_target(e);

        lv_area_t bar_area;
        lv_obj_get_coords(obj, &bar_area);

        // Рассчитываем прогресс в процентах
        float progress = static_cast<float>(self->value) / 10000.0f;

        // Определяем длину каждой стороны рамки
        int32_t total_length = 2 * (lv_area_get_width(&bar_area) + lv_area_get_height(&bar_area)) - 8 * self->radius;
        int32_t progress_length = static_cast<int32_t>(total_length * progress);

        lv_color_t progress_color = lv_color_hex(0x0000FF);

        // Рисуем рамку с прогрессом
        self->draw_progress(obj, progress_length, progress_color);
    }

    // Метод для рисования прогресса
    void draw_progress(lv_obj_t* obj, int32_t progress_length, lv_color_t color) {
        lv_area_t bar_area;
        lv_obj_get_coords(obj, &bar_area);

        // Настройки рисования прямоугольника
        lv_draw_rect_dsc_t rect_dsc;
        lv_draw_rect_dsc_init(&rect_dsc);
        rect_dsc.bg_color = color;
        rect_dsc.bg_opa = LV_OPA_COVER;
        rect_dsc.radius = radius;

        // Рисуем прогресс вдоль границ прямоугольника по часовой стрелке
        if (progress_length > 0) {
            // Верхняя сторона
            lv_area_t top_area = {bar_area.x1 + radius, bar_area.y1, bar_area.x1 + radius + progress_length, bar_area.y1 + indicator_width};
            if (top_area.x2 > bar_area.x2 - radius) top_area.x2 = bar_area.x2 - radius;
            lv_draw_rect(&top_area, &rect_dsc);
            progress_length -= (top_area.x2 - top_area.x1);

            // Правая сторона
            if (progress_length > 0) {
                lv_area_t right_area = {bar_area.x2 - indicator_width, bar_area.y1 + radius, bar_area.x2, bar_area.y1 + radius + progress_length};
                if (right_area.y2 > bar_area.y2 - radius) right_area.y2 = bar_area.y2 - radius;
                lv_draw_rect(&right_area, &rect_dsc);
                progress_length -= (right_area.y2 - right_area.y1);
            }

            // Нижняя сторона
            if (progress_length > 0) {
                lv_area_t bottom_area = {bar_area.x2 - radius - progress_length, bar_area.y2 - indicator_width, bar_area.x2 - radius, bar_area.y2};
                if (bottom_area.x1 < bar_area.x1 + radius) bottom_area.x1 = bar_area.x1 + radius;
                lv_draw_rect(&bottom_area, &rect_dsc);
                progress_length -= (bottom_area.x2 - bottom_area.x1);
            }

            // Левая сторона
            if (progress_length > 0) {
                lv_area_t left_area = {bar_area.x1, bar_area.y2 - radius - progress_length, bar_area.x1 + indicator_width, bar_area.y2 - radius};
                if (left_area.y1 < bar_area.y1 + radius) left_area.y1 = bar_area.y1 + radius;
                lv_draw_rect(&left_area, &rect_dsc);
            }
        }

        // Рисуем скругления углов
        draw_corner_arcs(obj, progress_length, color);
    }

    // Метод для рисования углов
    void draw_corner_arcs(lv_obj_t* obj, int32_t progress_length, lv_color_t color) {
        lv_area_t bar_area;
        lv_obj_get_coords(obj, &bar_area);

        // Настройки рисования дуг
        lv_draw_arc_dsc_t arc_dsc;
        lv_draw_arc_dsc_init(&arc_dsc);
        arc_dsc.color = color;
        arc_dsc.width = indicator_width;

        // Верхний левый угол
        lv_draw_arc(bar_area.x1 + radius, bar_area.y1 + radius, radius, 180, 270, &arc_dsc);

        // Верхний правый угол
        lv_draw_arc(bar_area.x2 - radius, bar_area.y1 + radius, radius, 270, 360, &arc_dsc);

        // Нижний правый угол
        lv_draw_arc(bar_area.x2 - radius, bar_area.y2 - radius, radius, 0, 90, &arc_dsc);

        // Нижний левый угол
        lv_draw_arc(bar_area.x1 + radius, bar_area.y2 - radius, radius, 90, 180, &arc_dsc);
    }
};

#endif