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
public:
    static const std::map<std::string, std::vector<std::string>> translations;
    uint8_t currentLanguage = 0;

    Translator() {}

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