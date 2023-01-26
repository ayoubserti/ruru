

#include "pch.h"
#include "ruru.h"
#include "table.h"
#include "database.h"

namespace ruru
{

    Database::Database(const std::string &name) : name(name) {}


    Database* Database::newDatabase(const std::string &name)
    {
        return new Database(name);
    }

    void Database::addTable(Table *table, StorageEngine *storageEngine)
    {
        tables[table->getName()] = table;
        storageEngines[table->getName()] = storageEngine;
    }

    Table *Database::getTable(const std::string &tableName)
    {
        auto it = tables.find(tableName);
        if (it == tables.end())
            return nullptr;
        else
            return it->second;
    }

    std::vector<Table *> Database::getAllTables()
    {
        std::vector<Table *> res;
        for (auto &[name, table] : tables)
        {
            res.push_back(table);
        }
        return res;
    }

    void Database::removeTable(const std::string &tableName)
    {
        tables.erase(tableName);
        storageEngines.erase(tableName);
    }

    StorageEngine *Database::getStorageEngine(const std::string &tableName)
    {
        auto it = storageEngines.find(tableName);
        if (it == storageEngines.end())
            return nullptr;
        else
            return it->second;
    }

}