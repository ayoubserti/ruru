// Copyright (c) 2023 Ayoub Serti
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "pch.h"
#include "ruru.h"
#include "internal/basic_store_cache.h"

int main ()
{
    using namespace ruru::internal;

    CacheStore * cache = new CacheStore("test/db1/Employee.ru");
    cache->Load();

    return 0;
} 