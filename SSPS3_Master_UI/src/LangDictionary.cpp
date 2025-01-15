#include "LangDictionary.hpp"

const size_t wordEntriesCount = 68;

const std::vector<WordEntry> wordEntries PROGMEM = {
    {LANG_NAME, {"Русский", "Englsih", "Polska"}},
    /*"[раздача_инфо_1]"*/ {DOSING_INFO_1, {"Выбор", "Select", "Wybór"}},
    /*"[раздача_инфо_2]"*/ {DOSING_INFO_2, {"- / +", "- / +", "- / +"}},
    /*"[раздача_инфо_3]"*/ {DOSING_INFO_3, {"Отмена или\nНазад", "Cancel or\ngo back", "Anuluj lub\nwróć"}},
    /*"[раздача_инфо_4]"*/ {DOSING_INFO_4, {"Старт\nраздачи", "Pump\nstart", "Start\npompy"}},
    /*"[шаблоны_раздачи]"*/ {DOSING_TEMPLATES, {"Шаблоны раздачи", "Dispensing templates", "Opcje dozowania"}},
    /*"[литр_буква]"*/ {L_LETTER, {"л.", "l.", "l."}},
    /*"[бар_об_мин]"*/ {BAR_RPM, {"об/мин", "rpm", "obr/min"}},
    /*"[бар_темп_ц_молоко]"*/ {BAR_TEMPC_MILK, {"молоко °C", "dairy °C", "mleko °C"}},
    /*"[бар_рубашка]"*/ {BAR_TEMPC_JACKET, {"рубашка", "w.jacket", "w.kurtka"}},
    /*"[бар_батарея]"*/ {BAR_BATTERY, {"батарея", "battery", "bateria"}},
    /*"[рубашка_набор]"*/ {JACKET_FILLING, {"набор", "intake", "pobór"}},
    /*"[рубашка_готов]"*/ {JACKET_DONE, {"заполн.", "full", "pełna"}},
    /*"[рубашка_охлажд]"*/ {JACKET_COOLING, {"охлажд.", "chill", "chłodz."}},
    /*"[рубашка_380в]"*/ {JACKET_AWAIT_380V, {"сеть?", "power?", "power?"}},
    /*"[рубашка_пусто]"*/ {JACKET_EMPTY, {"пусто", "empty", "pusty"}},
    /*"[аккум_зарядка]"*/ {BAT_CHARGERING, {"зарядка", "charge", "ładowar."}},
    /*"[аккум_ошибка]"*/ {BAT_ERROR, {"ошибка", "error", "błąd"}},
    /*"[да]"*/ {YES, {"да", "yes", "tak"}},
    /*"[нет]"*/ {NO, {"нет", "no", "nie"}},
    /*"[имя_пастер]"*/ {NAME_PASTEUR, {"Пастеризация", "Pasteurisation", "Pasteryzacja"}},
    /*"[имя_нагрев]"*/ {NAME_HEATING, {"Подогрев", "Heating", "Ogrzewanie"}},
    /*"[имя_охлажд]"*/ {NAME_COOLING, {"Охлаждение", "Chilling", "Chłodzenie"}},
    /*"[имя_автопрог]"*/ {NAME_AUTOPROG, {"Автопрог", "Auto prog.", "Prog. auto"}},
    /*"[имя_сыр_пармезан]"*/ {CHZ_PARMESAN, {"сыр \"Пармезан\"", "cheese \"Parmesan\"", "ser \"Parmezan\""}},
    /*"[имя_сыр_адыгейский]"*/ {CHZ_ADIGEJ, {"сыр \"Адыгейский\"", "cheese \"Adygei\"", "ser \"Adygei\""}},
    /*"[имя_сыр_тильзитский]"*/ {CHZ_TILSIT, {"сыр \"Тильзитский\"", "cheese \"Tilsiter\"", "ser \"Tilsiter\""}},
    /*"[имя_сыроварня_йогурт]"*/ {CHZ_YOGURT, {"йогурт", "yogurt", "jogurt"}},
    /*"[имя_сыроварня_творог]"*/ {CHZ_COT_CHEZ, {"творог", "cottage cheese", "Twarog"}},
    /*"[имя_сыр_свой]"*/ {CHZ_OWN_REC, {"свой сыр", "own recipe", "własny przepis"}},
    /*"[нет_поддержки_пастера]"*/ {PASTEUR_DOESNT_SUPPORTED, {"Оборудование не поддерживает пастеризацию", "The equipment does not support pasteurisation", "Urządzenie nie obsługuje pasteryzacji"}},
    /*"[дата_и_время]"*/ {DATE_TIME, {"Дата / Время", "Date / Time", "Data / Godzina"}},
    /*"[установить_время]"*/ {SETTINGS_DATE_TIME, {"Установка времени", "Setting time", "Ustawianie czasu"}},
    /*"[час]"*/ {HOUR, {"час", "hh", "gdz"}},
    /*"[мин]"*/ {MIN, {"мин", "mm", "min"}},
    /*"[сек]"*/ {SEC, {"сек", "ss", "sek"}},
    /*"[день]"*/ {DAY, {"день", "dd", "dz"}},
    /*"[мес]"*/ {MONTH, {"мес", "MM", "msc"}},
    /*"[год]"*/ {YEAR, {"год", "year", "rok"}},
    /*"[применить_ок]"*/ {OK_BUTTON, {"Применить", "Apply", "Zastosuj"}},
    /*"[настройки_раздача]"*/ {SETTINGS_DOSAGE, {"Раздача жидкости", "Liquid distribution", "Dystrybucja cieczy"}},
    /*"[настройки_калибровка]"*/ {SETTINGS_CALIBR, {"Калибровка насоса", "Pump calibration", "Kalibracja pompy"}},
    /*"[настройки_доп_л_м]"*/ {SETTINGS_VAL_L_M, {"доп. л/м", "additional l/min", "dodatkowe l/min"}},
    /*"[задача_пастеризация]"*/ {TASK_PASTEUR, {"программа \"Пастеризация\"", "program \"Pasteurization\"", "program \"Pasteryzacja\""}},
    /*"[задача_охлаждение]"*/ {TASK_COOLING, {"программа \"Охлаждение\"", "program \"Cooling\"", "program \"Chłodzenie\""}},
    /*"[задача_нагрев]"*/ {TASK_HEATING, {"программа \"Нагрев\"", "program \"Heating\"", "program \"Ogrzewanie\""}},
    /*"[пауза]"*/ {PAUSE, {"пауза", "pause", "pauza"}},
    /*"[ВКЛ]"*/ {ON, {"ВКЛ", "ON", "WŁ."}},
    /*"[ВЫКЛ]"*/ {OFF, {"ВЫКЛ", "OFF", "WYŁ."}},
    /*"[задача_авто]"*/ {TASK_AUTOPROG, {"Авто прогр. #", "Auto program #", "Auto program #"}},
    /*"[задача_авто_номер]"*/ {TASK_AUTOPTOG_NUM, {"Авто ВКЛ прогр. #", "Auto ON program #", "Auto WŁ program #"}},
    /*"[настройки_будильник]"*/ {SETTINGS_WATCHDOG, {"Настр. включения по времени", "Timer setting", "Ustawienie czasowe"}},
    /*"[статус_запущено]"*/ {STATE_RUNNED, {"Запущено", "Running", "Uruchomiono"}},
    /*"[этап_1]"*/ {STAGE_1, {"1. пастер", "1. pasteur.", "1. pasteryz."}},
    /*"[этап_2]"*/ {STAGE_2, {"2. охлажд", "2. cooling", "2. chłodz."}},
    /*"[этап_3_сыр]"*/ {STAGE_3_CHM, {"3. резка", "3. cutting", "3. cięcie"}},
    /*"[этап_3_тмп]"*/ {STAGE_3_TMPE, {"3. нагрев", "3. heating", "3. ogrzewan."}},
    /*"[этап_4]"*/ {STAGE_4, {"4. замеш.", "4. mixing", "4. mieszanie"}},
    /*"[этап_5]"*/ {STAGE_5, {"5. нагрев", "5. heating", "5. ogrzewan."}},
    /*"[этап_6]"*/ {STAGE_6, {"6. сушка", "6. drying", "6. suszenie"}},
    /*"[задача_своя]"*/ {TASK_OWN, {"свой рецепт #", "custom recipe #", "własny przepis #"}},
    /*"[настройка_шаблона]"*/ {EDIT_TEMPLATE, {"Поэтапная настройка", "Step-by-step setting", "Ustawienia krok po kroku"}},
    /*"[этап_набор_воды]"*/ {STAGE_WJACKET_FILLING, {"Набор воды", "Water intake", "Pobór wody"}},
    /*"[этап_пастеризация]"*/ {STAGE_PASTEUR, {"Пастеризация", "Pasteurization", "Pasteryzacja"}},
    /*"[этап_охлаждение]"*/ {STAGE_COOLING, {"Охлаждение", "Cooling", "Chłodzenie"}},
    /*"[этап_резка]"*/ {STAGE_CUTTING, {"Резка", "Cutting", "Cięcie"}},
    /*"[этап_замешивание]"*/ {STAGE_MIXING, {"Замешивание", "Mixing", "Mieszanie"}},
    /*"[этап_нагрев]"*/ {STAGE_HEATING, {"Нагрев", "Heating", "Ogrzewanie"}},
    /*"[этап_сушка]"*/ {STAGE_DRYING, {"Сушка", "Drying", "Suszenie"}},
    /*"[этап_выдержка]"*/ {STAGE_EXPOSURE, {"Выдержка", "Maturation", "Dojrzewanie"}},
    /*"[этап_подгонка_температуры]"*/ {STAGE_TO_AIM, {"*подгонка температуры*", "*temperature adjustment*", "*dostosowanie temperatury*"}},
    /*"[ожидание_нажмите_кнопку]"*/ {STAGE_AWAIT_USER, {"Ожидание: нажмите кнопку", "Waiting: press button", "Oczekiwanie: naciśnij przycisk"}}
};