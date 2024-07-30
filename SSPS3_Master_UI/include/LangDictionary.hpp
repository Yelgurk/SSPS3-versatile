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
            {"[аккум_зарядка]", {"зарядка", "charge", "ładowar.", "cargad."}},
            {"[аккум_ошибка]", {"ошибка", "error", "błąd", "error"}},
            {"[да]", {"да", "yes", "tak", "sí"}},
            {"[нет]", {"нет", "no", "nie", "no"}},
            {"[имя_пастер]", {"Пастеризация", "Pasteurisation", "Pasteryzacja", "Pasteurización"}},
            {"[имя_нагрев]", {"Подогрев", "Heating", "Ogrzewanie", "Calefacción"}},
            {"[имя_охлажд]", {"Охлаждение", "Chilling", "Chłodzenie", "Escalofriante"}},
            {"[имя_автопрог]", {"Автопрог", "Auto prog.", "Prog. auto", "Prog. auto"}},
            {"[имя_сыр_пармезан]", {"сыр \"Пармезан\"", "cheese \"Parmesan\"", "ser \"Parmezan\"", "queso \"Parmesano\""}},
            {"[имя_сыр_адыгейский]", {"сыр \"Адыгейский\"", "cheese \"Adygei\"", "ser \"Adygei\"", "queso \"Adygei\""}},
            {"[имя_сыр_тильзитский]", {"сыр \"Тильзитский\"", "cheese \"Tilsiter\"", "ser \"Tilsiter\"", "queso \"Tilsiter\""}},
            {"[имя_сыр_свой]", {"свой сыр", "own recipe", "własny przepis", "receta propia"}},
            {"[нет_поддержки_пастера]", {
                "Оборудование не поддерживает пастеризацию",
                "The equipment does not support pasteurisation",
                "Urządzenie nie obsługuje pasteryzacji",
                "El equipo no admite la pasteurización"
            }},
            {"[дата_и_время]", {"Дата / Время", "Date / Time", "Data / Godzina", "Fecha / Hora"}},
            {"[установить_время]", {"Установка времени", "Setting time", "Ustawianie czasu", "Ajustar la hora"}},
            {"[час]", {"час", "hh", "gdz", "hora"}},
            {"[мин]", {"мин", "mm", "min", "min"}},
            {"[сек]", {"сек", "ss", "sek", "seg"}},
            {"[день]", {"день", "dd", "dz", "día"}},
            {"[мес]", {"мес", "MM", "msc", "mes"}},
            {"[год]", {"год", "year", "rok", "año"}},
            {"[применить_ок]", {"Применить", "Apply", "Zastosuj", "Aplicar"}},
            {"[настройки_раздача]", {"Раздача жидкости", "Liquid distribution", "Dystrybucja cieczy", "Distribución de líquidos"}},
            {"[настройки_калибровка]", {"Калибровка насоса", "Pump calibration", "Kalibracja pompy", "Calibración de la bomba"}},
            {"[настройки_доп_л_м]", {"доп. л/м", "additional l/min", "dodatkowe l/min", "l/min adicional"}},
            {"[задача_пастеризация]", {"программа \"Пастеризация\"", "program \"Pasteurization\"", "program \"Pasteryzacja\"", "programa \"Pasteurización\""}},
            {"[задача_охлаждение]", {"программа \"Охлаждение\"", "program \"Cooling\"", "program \"Chłodzenie\"", "programa \"Enfriamiento\""}},
            {"[задача_нагрев]", {"программа \"Нагрев\"", "program \"Heating\"", "program \"Ogrzewanie\"", "programa \"Calentamiento\""}},
            {"[пауза]", {"пауза", "pause", "pauza", "pausa"}},
            {"[ВКЛ]", {"ВКЛ", "ON", "WŁ.", "ENC."}},
            {"[ВЫКЛ]", {"ВЫКЛ", "OFF", "WYŁ.", "APAG."}},
            {"[задача_авто]", {"Авто прогр. #", "Auto program #", "Auto program #", "Auto programa #"}},
            {"[задача_авто_номер]", {"Авто ВКЛ прогр. #", "Auto ON program #", "Auto WŁ program #", "Auto ENC programa #"}},
            {"[настройки_будильник]", {"Настр. включения по времени", "Timer setting", "Ustawienie czasowe", "Configuración del temporizador"}},
            {"[статус_запущено]", {"Запущено", "Running", "Uruchomiono", "En ejecución"}},
            {"[этап_1]", {"1. пастер", "1. pasteurization", "1. pasteryzacja", "1. pasteurización"}},
            {"[этап_2]", {"2. охлажд", "2. cooling", "2. chłodzenie", "2. enfriamiento"}},
            {"[этап_3_сыр]", {"3. резка", "3. cutting", "3. cięcie", "3. corte"}},
            {"[этап_3_тмп]", {"3. нагрев", "3. heating", "3. ogrzewanie", "3. calentamiento"}},
            {"[этап_4]", {"4. замеш.", "4. mixing", "4. mieszanie", "4. mezcla"}},
            {"[этап_5]", {"5. нагрев", "5. heating", "5. ogrzewanie", "5. calentamiento"}},
            {"[этап_6]", {"6. сушка", "6. drying", "6. suszenie", "6. secado"}},
            {"[задача_своя]", {"свой рецепт #", "custom recipe #", "własny przepis #", "receta personalizada #"}},
            {"[настройка_шаблона]", {"Поэтапная настройка", "Step-by-step setting", "Ustawienia krok po kroku", "Configuración paso a paso"}},
            {"[этап_набор_воды]", {"Набор воды", "Water intake", "Pobór wody", "Toma de agua"}},
            {"[этап_пастеризация]", {"Пастеризация", "Pasteurization", "Pasteryzacja", "Pasteurización"}},
            {"[этап_охлаждение]", {"Охлаждение", "Cooling", "Chłodzenie", "Enfriamiento"}},
            {"[этап_резка]", {"Резка", "Cutting", "Cięcie", "Corte"}},
            {"[этап_замешивание]", {"Замешивание", "Mixing", "Mieszanie", "Mezcla"}},
            {"[этап_нагрев]", {"Нагрев", "Heating", "Ogrzewanie", "Calentamiento"}},
            {"[этап_сушка]", {"Сушка", "Drying", "Suszenie", "Secado"}},
            {"[этап_выдержка]", {"Выдержка", "Maturation", "Dojrzewanie", "Maduración"}},
            {"[этап_подгонка]", {"*подгонка температуры*", "*temperature adjustment*", "*dostosowanie temperatury*", "*ajuste de temperatura*"}},
            {"[этап_ожидание]", {"Ожидание: нажмите кнопку", "Waiting: press button", "Oczekiwanie: naciśnij przycisk", "Esperando: presione el botón"}}
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