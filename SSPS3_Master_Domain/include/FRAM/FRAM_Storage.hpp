#pragma once
#ifndef FRAM_Storage_hpp
#define FRAM_Storage_hpp

#include <Arduino.h>
#include <Wire.h>
#include <map>
#include <string>
#include <memory>
#include <any>
#include <memory>
#include "FRAM_Object.hpp"

using namespace std;
using FramMap = std::map<std::string, std::unique_ptr<FRAMObjectBase>>;

extern uint8_t FRAM_address;
extern TwoWire * itcw;

template<typename T, typename... Args>
std::unique_ptr<T> custom_make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

extern uint16_t currentAddress;
extern FramMap addressMap;

class Storage
{
public:
    template<typename T>
    static FRAMObject<T>& allocate(T default_value, bool is_resetable, uint32_t new_addr, size_t size) {
        return Storage::allocate(default_value, is_resetable, new_addr, size);
    }

    template<typename T>
    static FRAMObject<T>& allocate(T default_value, bool is_resetable, uint32_t new_addr, std::string name) {
        return Storage::allocate(default_value, is_resetable, new_addr, sizeof(T), name);
    }

    template<typename T>
    static FRAMObject<T>& allocate(T default_value, bool is_resetable = true, uint32_t new_addr = 0, size_t size = sizeof(T), std::string name = "_")
    {
        currentAddress = new_addr > currentAddress ? new_addr : currentAddress;
        uint16_t address = currentAddress;
        currentAddress += (size + 1);

        auto obj = custom_make_unique<FRAMObject<T>>(address, default_value, size);
        FRAMObject<T>* ptr = obj.get();
        ptr->set_is_resetable(is_resetable);

        name = name == "_" ? ("v_" + to_string(address)) : name;  
        
        addressMap[name] = std::move(obj);
        return *ptr;
    }

    static FRAMObjectBase* getObject(const std::string& name)
    {
        if (addressMap.find(name) != addressMap.end())
            return &*addressMap[name];
        else
            return nullptr;
    }

    static void reset_all(bool hard_reset = false)
    {
        for (auto& pair : addressMap)
            if (!pair.second->get_is_system_val() && (pair.second->get_is_resetable() || hard_reset))
                    pair.second->reset();
    }

    static void first_boot()
    {
        for (auto& pair : addressMap)
            pair.second->reset();
    }

    static void print_list_of_addr()
    {
        for (auto& pair : addressMap)
        {
            pair.second->print_addr();
            Serial.print("(");
            pair.second->print_size();
            Serial.print(") -> ");
            pair.second->print_end_addr();
            Serial.println();
        }
    }
};

#endif