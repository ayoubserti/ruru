

#include "pch.h"
#include "ruru.h"
#include "table.h"
#include "database.h"
#include "storage_engine.h"
#include "record.h"
#include "resultset.h"
namespace ruru
{
    // private functions
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

    Database::Database(const std::string &name) : name(name), schema(nullptr) {}

    void Database::_initSchemaDB()
    {
        assert(schema == nullptr);
        schema.reset(new Database(name));

        Table *schemaTable = new Table("__schema", schema.get());
        Column object_name("object_name", DataTypes::eVarChar);     // name
        Column object_kind("object_kind", DataTypes::eVarChar);     // table, column, index
        Column object_type("object_type", DataTypes::eVarChar);     // data type
        Column object_parent("object_parent", DataTypes::eVarChar); // parent: table or null
        schemaTable->addColumn(object_name);
        schemaTable->addColumn(object_kind);
        schemaTable->addColumn(object_type);
        schemaTable->addColumn(object_parent);
        StorageEngine *schemaStore = new StorageEngine(name, true);
        schema->addTable(schemaTable, schemaStore);
    }

    Database *Database::newDatabase(const std::string &name)
    {
        Database *db = new Database(name);
        db->_initSchemaDB();
        return db;
    }

    Database *Database::openDatabase(const std::filesystem::path &path)
    {
        /*
        database folder structure:
        - folder name = database name
        - structure is stored into a table file (schema.ru)
        - schema.ru stores tables as follow:

        - each table structure is a record
        - json dump inside (schema.json)
        - each table have it's data file (<table_name>.ru)
        */
        Database *db = new Database(path.filename());
        db->_initSchemaDB();
        StorageEngine *schemaStore = db->schema->getStorageEngine("__schema");
        Table *schemaTable = db->schema->getTable("__schema");
        std::vector<Record> all_tables_structrue = schemaStore->SelectAll();

        for (auto it = all_tables_structrue.begin(); it != all_tables_structrue.end(); ++it)
        {
            // for simplicity purpose, we consider that schema was stored in the store file in sequence, in the correct order
            //  the implementation of relations will remove this caveat
            assert(it->fields_.size() % 4 == 0); // debugging
            RecordTable *rec = schemaTable->GetRecord(it->row_id_);
            std::string xname;
            rec->GetFieldValue("object_name", xname);
            std::string xkind;
            rec->GetFieldValue("object_kind", xkind);
            std::string xtype;
            rec->GetFieldValue("object_type", xtype);
            std::string xparent;
            rec->GetFieldValue("object_parent", xparent);
            if (xkind == "TABLE")
            {
                Table *tbl = new Table(xname, db);
                StorageEngine *store = new StorageEngine(xname + db_extension);
                db->addTable(tbl, store);
            }
            else if (xkind == "COLUMN")
            {
                Table *tbl = db->getTable(xparent);
                Column cl(xname, getTypeFromString(xtype));
                tbl->addColumn(cl);
            }
            else
            {
                assert(false && "NOT IMPLEMENTED");
            }
        }
        return db;
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

    bool Database::saveSchema(const std::filesystem::path &path)
    {
        bool result = true;
        auto tbl_schema = schema->getTable("__schema");
        for (auto &&tbl : tables)
        {
            auto rec = tbl_schema->CreateRecord();
            rec->SetFieldValue("object_name", tbl.first);
            rec->SetFieldValue("object_kind", "TABLE");
            rec->SetFieldValue("object_type", "");
            rec->SetFieldValue("object_parent", "");
            rec->Save();
            delete rec;
            const auto &cols = tbl.second->getColumns();
            for (const auto &it : cols)
            {
                auto rec = tbl_schema->CreateRecord();
                rec->SetFieldValue("object_name", it.getName());
                rec->SetFieldValue("object_kind", "COLUMN");
                rec->SetFieldValue("object_type", getStringFromType(it.getType()));
                rec->SetFieldValue("object_parent", tbl.first);
                rec->Save();
                delete rec;
            }
        }

        return result;
    }

    Database::~Database()
    {
        for (auto &&it : tables)
            delete it.second;
        for (auto &&it : storageEngines)
            delete it.second;
    }
}
