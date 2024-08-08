#ifndef TRANSLATOR_HPP
#define TRANSLATOR_HPP

#include <Arduino.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>

enum UI_LANGUAGE : uint8_t
{
    RUSSIAN,
    ENGLISH,
    POLISH,
    _END
};

enum UI_WORD_KEY : uint16_t
{
    LANG_NAME,
    DOSING_INFO_1,
    DOSING_INFO_2,
    DOSING_INFO_3,
    DOSING_INFO_4,
    DOSING_TEMPLATES,
    L_LETTER,
    BAR_RPM,
    BAR_TEMPC_MILK,
    BAR_TEMPC_JACKET,
    BAR_BATTERY,
    JACKET_FILLING,
    JACKET_DONE,
    JACKET_COOLING,
    JACKET_EMPTY,
    BAT_CHARGERING,
    BAT_ERROR,
    YES,
    NO,
    NAME_PASTEUR,
    NAME_HEATING,
    NAME_COOLING,
    NAME_AUTOPROG,
    CHZ_PARMESAN,
    CHZ_ADIGEJ,
    CHZ_TILSIT,
    CHZ_OWN_REC,
    PASTEUR_DOESNT_SUPPORTED,
    DATE_TIME,
    SETTINGS_DATE_TIME,
    HOUR,
    MIN,
    SEC,
    DAY,
    MONTH,
    YEAR,
    OK_BUTTON,
    SETTINGS_DOSAGE,
    SETTINGS_CALIBR,
    SETTINGS_VAL_L_M,
    TASK_PASTEUR,
    TASK_COOLING,
    TASK_HEATING,
    PAUSE,
    ON,
    OFF,
    TASK_AUTOPROG,
    TASK_AUTOPTOG_NUM,
    SETTINGS_WATCHDOG,
    STATE_RUNNED,
    STAGE_1,
    STAGE_2,
    STAGE_3_CHM,
    STAGE_3_TMPE,
    STAGE_4,
    STAGE_5,
    STAGE_6,
    TASK_OWN,
    EDIT_TEMPLATE,
    STAGE_WJACKET_FILLING,
    STAGE_PASTEUR,
    STAGE_COOLING,
    STAGE_CUTTING,
    STAGE_MIXING,
    STAGE_HEATING,
    STAGE_DRYING,
    STAGE_EXPOSURE,
    STAGE_TO_AIM,
    STAGE_AWAIT_USER,
    _WORD_KEY_END
};

struct WordEntry
{
    UI_WORD_KEY key;
    const char* translations[UI_LANGUAGE::_END];
};

extern const std::vector<WordEntry> wordEntries;

class Translator {
private:
    uint8_t currentLanguage;

protected:
    void _set_lang(uint8_t lang) {
        this->currentLanguage = lang;
    }

public:
    Translator() {
        currentLanguage = UI_LANGUAGE::RUSSIAN;
    }

    static Translator& instance()
    {
        static Translator inst;
        return inst;
    }

    uint8_t get_lang() {
        return this->currentLanguage;
    }

    static void set_lang(uint8_t lang)
    {
        if (lang < UI_LANGUAGE::_END)
            instance()._set_lang(lang);
    }

    static std::string get(UI_WORD_KEY key) {
        return get(key, instance().get_lang());
    }

    static std::string get(UI_WORD_KEY key, uint8_t lang)
    {
        if (lang >= UI_LANGUAGE::_END)
        return "UI_ERR";

        for (size_t i = 0; i < wordEntries.size(); ++i)
        {
            WordEntry entry;
            memcpy_P(&entry, &wordEntries[i], sizeof(WordEntry));

            if (entry.key == key)
                return std::string(entry.translations[lang]);
        }
        return "UI_ERR";
    }
};

#endif