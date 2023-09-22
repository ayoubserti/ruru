// Copyright (c) 2023 Ayoub Serti
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "pch.h"
#include "ruru.h"
#include "internal/tools.h"
#include "record.h"
using namespace ruru;

bool _ApplyFilter(const Record &rec, const Filter &filter)
{
    // quick & dirty solution
    //  we may consider to create operator functions
    bool result = true;
    Field fl = rec.fields_[filter.column_indx];
    switch (fl.type_)
    {
    case DataTypes::eInteger:
    {
        int64_t value = *reinterpret_cast<int64_t *>(fl.value_.get());
        return filter.Apply(value);
    }
    break;
    case DataTypes::eDouble:
    {
        double value = *reinterpret_cast<double *>(fl.value_.get());
        return filter.Apply(value);
    }

    break;
    case DataTypes::eVarChar:
    {
        std::string value = fl.value_.get();
        return filter.Apply(value);
    }

    break;
    case DataTypes::eNull:
    //TODO implement (is null, is not null ) filters
    break;
    default:
        throw new std::exception();
    }
    return result;
}