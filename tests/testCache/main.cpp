// Copyright (c) 2023 Ayoub Serti
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "pch.h"
#include "ruru.h"
#include "internal/basic_store_cache.h"
#include "record.h"

#include <iostream>

static void print_record ( const ruru::Record record)
{
    std::cout << "Record contains " << record.fields_.size() << " field\n";
    std::cout << "------------------------------------------------------\n";
    for ( auto&& f : record.fields_)
    {
        if ( f.type_ == ruru::DataTypes::eInteger)
        {
           auto value = *reinterpret_cast<int64_t *>(f.value_.get());
           std::cout << value << "\n";
        }
        else if ( f.type_ == ruru::DataTypes::eDouble)
        {
           auto value = *reinterpret_cast<double *>(f.value_.get());
           std::cout << value << "\n";
        }
        else if ( f.type_ == ruru::DataTypes::eVarChar )
        {
            
            uint64_t *len = reinterpret_cast<uint64_t *>(f.value_.get());
            if (*len == 0)
                std::cout << "<<EMPTY>>" << "\n";
            else
            {
                std::string value;
                value.assign(f.value_.get() + 8, *len);
                std::cout << "\"" << value  << "\""<< "\n";
            }
        }
        else if (f.type_ == ruru::DataTypes::eNull)
        {
            std::cout << "NULL\n";
        }
    }
    std::cout << "------------------------------------------------------\n";
}

int main ()
{
    using namespace ruru::internal;

    CacheStore * cache = new CacheStore("test/db1/Employee.ru");
    cache->Load();



    ruru::Filters_t filters;
    std::shared_ptr<ruru::Filter> filter1(new ruru::Filter(0, ruru::OperatorType::eGreater, 90, 0));
    filters.push_back(filter1);


    auto raws = cache->Lookup(filters);

    for (auto&& r : raws)
    {
        ruru::Record rec;
        if ( cache->GetRecord(r, rec) )
        {
            print_record(rec);
        }
    }

    cache->Flush();
    return 0;
} 