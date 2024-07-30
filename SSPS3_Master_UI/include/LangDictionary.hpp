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
    SPANISH,
    _END
};

class Translator
{
private:
    std::map<std::string, std::vector<std::string>> translations;
    uint8_t currentLanguage = 0;

    Translator()
    {
        translations = {
            {"[раздача_инфо_1]", {"Выбор", "Select", "Wybór", "Selección"}},
            {"[раздача_инфо_2]", {"- / +", "- / +", "- / +", "- / +"}},
            {"[раздача_инфо_3]", {"Отмена или\nНазад", "Cancel or\ngo back", "Anuluj lub\nwróć", "Cancelar\no volver"}},
            {"[раздача_инфо_4]", {"Старт\nраздачи", "Pump\nstart", "Start\npompy", "Arranque\nde la bomba"}},
            {"[шаблоны_раздачи]", {"Шаблоны раздачи", "Dispensing templates", "Opcje dozowania", "Posibles dosificaciones"}},
            {"[литр_буква]", {"л.", "l.", "l.", "l."}},
            {"[бар_об_мин]", {"об/мин", "rpm", "obr/min", "rpm"}},
            {"[бар_темп_ц_молоко]", {"молоко °C", "dairy °C", "mleko °C", "leche °C"}},
            {"[бар_рубашка]", {"рубашка", "w.jacket", "w.kurtka", "aqua"}},
            {"[бар_батарея]", {"батарея", "battery", "bateria", "batería"}},
            {"[рубашка_набор]", {"набор", "intake", "pobór", "inguesta"}},
            {"[рубашка_готов]", {"заполн.", "full", "pełna", "llena"}},
            {"[рубашка_охлажд]", {"охлажд.", "chill", "chłodz.", "refrig."}},
            {"[рубашка_пусто]", {"пусто", "empty", "pusty", "vacío"}},
            {"[аккум_зарядка]", {"зарядка", "", "", ""}},
            {"[аккум_ошибка]", {"ошибка", "", "", ""}},
            {"[да]", {"да", "", "", ""}},
            {"[нет]", {"нет", "", "", ""}},
            {"[имя_пастер]", {"Пастеризация", "", "", ""}},
            {"[имя_нагрев]", {"Подогрев", "", "", ""}},
            {"[имя_охлажд]", {"Охлаждение", "", "", ""}},
            {"[имя_автопрог]", {"Автопрог", "", "", ""}},
            {"[имя_сыр_пармезан]", {"сыр \"Пармезан\"", "", "", ""}},
            {"[имя_сыр_адыгейский]", {"сыр \"Адыгейский\"", "", "", ""}},
            {"[имя_сыр_тильзитский]", {"сыр \"Тильзитский\"", "", "", ""}},
            {"[имя_сыр_свой]", {"свой сыр", "", "", ""}},
            {"[нет_поддержки_пастера]", {"Оборудование не поддерживает пастеризацию", "", "", ""}},
            {"[дата_и_время]", {"Дата / Время", "Date / Time", "Data / Godzina", "Fecha / Hora"}},
            {"[установить_время]", {"Установка времени", "Setting time", "Ustawianie czasu", "Ajustar la hora"}},
            {"[час]", {"час", "hh", "gdz", "hora"}},
            {"[мин]", {"мин", "mm", "min", "min"}},
            {"[сек]", {"сек", "ss", "sek", "seg"}},
            {"[день]", {"день", "dd", "dz", "día"}},
            {"[мес]", {"мес", "MM", "msc", "mes"}},
            {"[год]", {"год", "year", "rok", "año"}},
            {"[применить_ок]", {"Применить", "Apply", "Zastosuj", "Aplicar"}}
        };
    }

public:
    static Translator& getInstance()
    {
        static Translator instance;
        return instance;
    }

    static void setLanguage(uint8_t lang)
    {
        if (lang < static_cast<uint8_t>(UI_LANGUAGE::_END))
            getInstance().currentLanguage = lang;
    }

    static void setLanguage(UI_LANGUAGE lang) {
        getInstance().currentLanguage = static_cast<uint8_t>(lang);
    }

    static std::string get(const std::string& key, const UI_LANGUAGE& lang) {
        return get(key, static_cast<uint8_t>(lang));
    }

    static std::string get(const std::string& key, const uint8_t& lang)
    {
        auto& instance = getInstance();
        auto it = instance.translations.find(key);
        if (it != instance.translations.end() && instance.currentLanguage < it->second.size())
            return it->second[lang >= static_cast<uint8_t>(UI_LANGUAGE::_END) ? static_cast<uint8_t>(UI_LANGUAGE::RUSSIAN) : lang];
        return "_=-*-=_";
    }

    static std::string get(const std::string& key)
    {
        auto& instance = getInstance();
        auto it = instance.translations.find(key);
        if (it != instance.translations.end() && instance.currentLanguage < it->second.size())
            return it->second[instance.currentLanguage];
        return "_=-*-=_";
    }
};