// Copyright (c) 2023 Ayoub Serti
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "pch.h"
#include "ruru.h"

#include <iostream>
#include <filesystem>

#include <nlohmann/json.hpp>

using namespace ruru;

auto newDatabase(std::filesystem::path path, const std::string &json_str)
{
    std::shared_ptr<IDatabase> ptr = IDatabase::newDatabase(path);

    ptr->setStorageEngineFactory(ruru::getEngineFactory(_basic_cached_factory));

    nlohmann::json json = nlohmann::json::parse(json_str);

    TablePtr tbl = ptr->newTable(json[0]["table_name"]);
    for (auto &&it : json[0]["columns"])
    {
        Column cl(it["column_name"], ::getTypeFromString(it["column_type"]));
        tbl->addColumn(cl);
    }

    ptr->saveSchema(path);
    return ptr;
}


int main(int argv, char **argc)
{
    // starting by Initializing ruru library
    ruru::Init();

    if (argv > 1)
    {
        std::string structure = R"(

        [ 
            {
                "table_name" : "Employee",
                "columns" :[
                    {
                        "column_name" : "ID",
                        "column_type" : "INTEGER"
                    },
                    {
                        "column_name" : "AGE",
                        "column_type" : "INTEGER"
                    },
                    {
                        "column_name" : "NAME",
                        "column_type" : "VARCHAR"
                    }
                ]
            }
        ]
        )";
        std::shared_ptr<IDatabase> db = newDatabase("test/db2/db2.ru", structure);
        TablePtr tbl = db->getTable("Employee");

        for ( int i = 0 ; i< 20; i++){
            RecordTablePtr rec = tbl->CreateRecord();
            rec->SetFieldValue("ID", (int64_t)i);
            int64_t age = i+15;
            rec->SetFieldValue("AGE" , age);
            rec->SetFieldValue("NAME", "AAA");
            rec->Save();
        }
        

        Filters_t filters;
        std::shared_ptr<Filter> filter1(new Filter(tbl->getColumnIndex("ID"), OperatorType::eLesser, 15, 0));
        filters.push_back(filter1);

        Column col2 = tbl->getColumn("AGE");
        std::shared_ptr<Filter> filter2(new Filter(tbl->getColumnIndex("AGE"), OperatorType::eGreaterOrEq, 37, 0));
        filters.push_back(filter2);

        ResultSetPtr res = tbl->Search(filters);

        auto r = res->First();
        if (r != nullptr)
        {
            std::string nn;
            r->GetFieldValue("NAME", nn);
            std::cout << nn << std::endl;
            while (!res->Eof())
            {
                r = res->Next();
                if (r != nullptr)
                {
                    r->GetFieldValue("NAME", nn);
                    std::cout << "Name " << nn << std::endl;
                    int64_t v;
                    r->GetFieldValue("AGE", v);
                    std::cout << "age " << v << std::endl;
                }
            }
        }

        //saveSchema(db.get(), "test/db1/db1.json");
    }
    else
    {
        //auto db = IDatabase::openDatabase("test/db1/db1.ru");
        //TablePtr tbl = db->getTable("Employee");
        
        auto db = IDatabase::openDatabase("test/newdb.ru");
        TablePtr tbl = db->getTable("MyTable");
        
        auto rec = tbl->GetRecord(16);

        std::cout << "Leaks : " << db.use_count() << "\n";
    }

    return 0;
}