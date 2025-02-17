#pragma once
#ifndef X_VAR_ARRAY_H
#define X_VAR_ARRAY_H

#ifdef DEV_SSPS3_RUN_ON_PLC
    #include "./x_var_allocator.h"
#else
    #include "x_var_allocator.h"
#endif

#include <array>
#include <functional>
#include <cassert>
#include <cstddef>
#include <utility>

template<typename T, std::size_t N>
class XVarArray
{
private:
    template<typename U>
    XVar<U>& _init(U default_value, signed int ext_mem_var_addr = 0, bool is_system_val = false, bool is_admin_val = false)
    {
        return XVarAllocator::instance()->allocate<U>(default_value, ext_mem_var_addr, is_system_val, is_admin_val);
    }

    std::array<std::reference_wrapper<XVar<T>>, N> data;
    XVar<T>* _selected = nullptr;
    int _selected_index = 0;

    std::function<void(XVar<T>&)> _callback;
    bool _infinity_selector = false;

public:
    // Variadic-конструктор: для каждого аргумента вызывается _init<T>()
    template<typename... Args, typename = std::enable_if_t<sizeof...(Args) == N>>
    XVarArray(Args&&... args)
        : data{ std::ref(_init<T>(std::forward<Args>(args)))... },
          _selected(nullptr),
          _infinity_selector(false),
          _callback(nullptr)
    {
        this->select(_selected_index);
    }

    XVar<T>& operator[](std::size_t idx) {
        return data[idx].get();
    }
    
    const XVar<T>& operator[](std::size_t idx) const {
        return data[idx].get();
    }
    
    void set_infinity_selector(bool flag) {
        _infinity_selector = flag;
    }
    
    void set_callback(const std::function<void(XVar<T>&)>& func) {
        _callback = func;
    }
    
    void select(int idx)
    {
        idx = idx < 0
            ? (!_infinity_selector ? 0 : static_cast<int>(N) - 1)
            : idx;

        idx = idx >= static_cast<int>(N)
            ? (!_infinity_selector ? static_cast<int>(N) - 1 : 0)
            : idx;

        
        _selected = &data[_selected_index = idx].get();
        
        if (_callback)
            _callback(*_selected);
    }
    
    void next() {
        this->select(++_selected_index);
    }

    void prev() {
        this->select(--_selected_index);
    }

    XVar<T>& get_selected() {
        return *_selected;
    }
};

#endif