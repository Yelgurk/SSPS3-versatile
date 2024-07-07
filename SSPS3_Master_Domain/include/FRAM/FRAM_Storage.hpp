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
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

class Storage
{
private:
    static uint16_t currentAddress;
    static FramMap addressMap;

public:
    template<typename T>
    static FRAMObject<T>& allocate(T default_value, uint32_t new_addr, size_t size) {
        return Storage::allocate(default_value, new_addr, size);
    }

    template<typename T>
    static FRAMObject<T>& allocate(T default_value, uint32_t new_addr, std::string name) {
        return Storage::allocate(default_value, new_addr, sizeof(T), name);
    }

    template<typename T>
    static FRAMObject<T>& allocate(T default_value, uint32_t new_addr = 0, size_t size = sizeof(T), std::string name = "_")
    {
        currentAddress = new_addr > currentAddress ? new_addr : currentAddress;
        uint16_t address = currentAddress;
        currentAddress += (size + 1);

        auto obj = make_unique<FRAMObject<T>>(address, default_value, size);
        FRAMObject<T>* ptr = obj.get();

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
};

uint16_t Storage::currentAddress = 0;
FramMap Storage::addressMap = FramMap();

#endif