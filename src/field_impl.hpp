// Copyright (c) 2023 Ayoub Serti
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "pch.h"
#include "record.h"
namespace ruru
{
template <typename T>
    void Field::SetValue(const T &value)
    {
    }

    template <>
    void Field::SetValue(const int64_t &value)
    {
        type_ = DataTypes::eInteger;
        value_.reset((char*)malloc(sizeof(value))); //TODO: use allocator
        memcpy(value_.get(),&value,sizeof(value));
    }

    template <>
    void Field::SetValue(const double &value)
    {
        type_ = DataTypes::eDouble;
        value_.reset((char*)malloc(sizeof(value))); //TODO: use allocator
        memcpy(value_.get(),&value,sizeof(value));
    }

    template <>
    void Field::SetValue(const std::string &value)
    {
        type_ = DataTypes::eVarChar;
        uint64_t len = value.size();
        value_.reset((char*)malloc(sizeof(uint64_t) + len));  //TODO: use allocator
        memcpy(value_.get() , &len , sizeof(len));
        memcpy(value_.get()+sizeof(len),value.c_str(),len);
    }

}

    