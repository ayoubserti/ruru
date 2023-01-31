#include "pch.h"
#include "ruru.h"

#include <iostream>
#include <filesystem>
#include <nlohmann/json.hpp>

using namespace ruru;

struct TypeMapping
{
    const std::string name;
    DataTypes type;
};

TypeMapping mappingList[] = {
    {"INTEGER", DataTypes::eInteger},
    {"VARCHAR", DataTypes::eVarChar},
    {"DECIMAL", DataTypes::eDouble},
    {"BYTES", DataTypes::eBinary}};

DataTypes getTypeFromString(const std::string &name)
{
    for (auto &&it : mappingList)
    {
        if (it.name == name)
            return it.type;
    }

    return DataTypes::eUnknown;
}

std::string getStringFromType(DataTypes type)
{
    for (auto &&it : mappingList)
    {
        if (it.type == type)
            return it.name;
    }

    return "";
}

auto openDatabase(std::filesystem::path path)
{
    std::shared_ptr<IDatabase>  ptr(nullptr);   
    if (!std::filesystem::is_regular_file(path))
        return ptr;
    
    ptr = IDatabase::newDatabase(path.filename());

    return ptr;
}

auto newDatabase(std::filesystem::path path, const std::string &json_str)
{
    std::shared_ptr<IDatabase> ptr = IDatabase::newDatabase(path);

    nlohmann::json json = nlohmann::json::parse(json_str);
   
    TablePtr tbl =  ptr->newTable(json[0]["table_name"]);
    for (auto &&it : json[0]["columns"])
    {
        Column cl(it["column_name"], getTypeFromString(it["column_type"]));
        tbl->addColumn(cl);
    }
    
    ptr->saveSchema(path);
    return ptr;
}

// save dabase schema into a file
void saveSchema(IDatabase *db, std::filesystem::path path)
{
    auto tbls = db->getAllTables();

    nlohmann::json json = nlohmann::json::array();
    for (auto &&it : tbls)
    {
        nlohmann::json obj = nlohmann::json({});
        const std::string &name = it->getName();
        obj["table_name"] = name;
        const std::vector<Column> cols = it->getColumns();
        nlohmann::json col_arr = nlohmann::json::array();
        for (auto &&it_col : cols)
        {
            nlohmann::json obj_col = nlohmann::json({});
            obj_col["column_name"] = it_col.getName();
            obj_col["column_type"] = getStringFromType(it_col.getType()); // TODO: store type string in column object
            col_arr.push_back(obj_col);
        }
        obj["columns"] = col_arr;
        json.push_back(obj);
    }
    std::ofstream file(path.c_str());
    if (file.is_open())
    {
        file << json;
    }
}

int main()
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
#if 1
    std::shared_ptr<IDatabase> db = newDatabase("test/db1/db1.ru", structure);
    TablePtr tbl = db->getTable("Employee");
    for (int i = 0; i < 10; i++)
    {
        RecordTablePtr rec = tbl->CreateRecord();
        int64_t id = 10 + i;
        rec->SetFieldValue("ID", id);
        int64_t age = 34 + i;
        rec->SetFieldValue("AGE", age);
        std::string n("Ayoub");
        n.append(i, 'A');
        rec->SetFieldValue("NAME", n);
        rec->Save();
        
    }
    Filters_t filters;
    std::shared_ptr<Filter> filter1(new Filter(tbl->getColumnIndex("ID"), OperatorType::eLesser, 15, 0));
    filters.push_back(filter1);

    Column col2 = tbl->getColumn("AGE");
    std::shared_ptr<Filter> filter2(new Filter( tbl->getColumnIndex("AGE"), OperatorType::eGreaterOrEq, 37, 0));
    filters.push_back(filter2);

    ResultSetPtr res = tbl->Search(filters);

    auto r = res->First();
    std::string nn;
    r->GetFieldValue("NAME", nn);
    std::cout << nn << std::endl;
    while (!res->Eof())
    {
        r = res->Next();
        if (r != nullptr)
        {
            r->GetFieldValue("NAME", nn);
            std::cout << nn << std::endl;
        }
    }

    saveSchema(db.get(), "test/db1/db1.json");
#else
    
    auto db = IDatabase::openDatabase("test/db1/db1.ru");
    TablePtr tbl = db->getTable("Employee");
    auto r = tbl->Search({});
#endif     
    std::cout << "Leaks : " << db.use_count() << "\n";
    return 0;
}