#include <Arduino.h>
#include <iostream>
#include <map>
#include <vector>

using namespace std;

enum class UI_LANGUAGE : uint8_t
{
    RUSSIAN,
    ENGLISH,
    POLISH,
    SPANISH
};

static std::map<string, vector<string>> const ui_lang_dict
{
    {"[язык]",                  {"Язык", "Language", "Jezyk", "Idioma"}},
    {"[температура]",           {"Температура", "Temperature", "Temperatura", "Temperatura"}},
    {"[приветствие]",           {"Вас приветствует Salutem!", "Salutem welcomes you!", "Salutem wita!", "Salutem le da la bienvenida!"}},
    {"[ошибка]",                {"Ошибка", "Error", "Blad", "Error"}},
    {"[старт]",                 {"Старт", "Start", "Start", "Start"}},
    {"[стоп]",                  {"Стоп", "Stop", "Stop", "Stop"}},
    {"[пауза]",                 {"Пауза", "Pause", "Pauza", "Pause"}},
    {"[раздача_молока]",        {"Рздача молока", "Milk dosage", "Dozowanie mleka", "Dosificación de la leche"}},
    {"[будильник]",             {"Будильник", "Alarm clock", "Budzik", "Despertador"}},
    {"[пастеризация]",          {"Пастеризация", "Pasteurization", "Pasteryzacja", "Pasteurización"}},
    {"[охлаждение]",            {"Охлаждение", "Cooling", "Chłodzenie", "Refrigeración"}},
    {"[нагрев]",                {"Нагрев", "Heating", "Ogrzewanie", "Calefaccion"}},
    {"[выдержка_по_времени]",   {"Выдержка", "Time exposure", "Czas ekspozycji", "Tiempo de exposición"}},
    {"[программа]",             {"Программа", "Program", "Program", "Programa"}},
    {"[шаблон]",                {"Шаблон программы", "Program template", "Szablon programu", "Plantilla del programa"}},
    {"[вкл]",                   {"ВКЛ", "On", "Wlacz", "On"}},
    {"[выкл]",                  {"ВЫКЛ", "Off", "Wylacz", "Off"}},
    {"[чч]",                    {"ЧЧ", "HH", "HH", "HH"}},
    {"[мм]",                    {"ММ", "MM", "MM", "MM"}},
    {"[сс]",                    {"СС", "SS", "SS", "SS"}},
    {"[уведомление]",           {"Уведомление", "Notification", "Powiadomienie", "Notificación"}},
    {"[меню]",                  {"Меню", "Menu", "Menu", "Menú"}},
    {"[назад]",                 {"Назад", "Back", "Powrót", "Volver"}},
    {"[ок]",                    {"Ок", "Ok", "Ok", "Ok"}}
};