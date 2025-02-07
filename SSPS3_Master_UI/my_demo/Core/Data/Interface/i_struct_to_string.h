#pragma once
#ifndef I_STRUCT_TO_STRING_H
#define I_STRUCT_TO_STRING_H

#include <string>

struct IStructToString
{
    virtual std::string to_string() = 0;
};

#endif // !I_STRUCT_TO_STRING_H
