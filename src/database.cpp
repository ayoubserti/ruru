

#include "pch.h"
#include "ruru.h"
#include "database.h"
#include "record.h"
#include "internal/basic_storage_engine.h"
namespace ruru
{
    //constants
    constexpr const char* __schema = "__schema";
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
    using internal::BasicStorageEngine;

#pragma region
    
    Database::Database(const std::filesystem::path &path) 
    :name(path.filename())
    ,path(path)
    ,schema(nullptr) {}

    void Database::_initSchemaDB()
    {
        assert(schema == nullptr);
        schema.reset(new Database(name));

        TablePtr schemaTable { new Table(__schema, schema)};
        Column object_name("object_name", DataTypes::eVarChar);     // name
        Column object_kind("object_kind", DataTypes::eVarChar);     // table, column, index
        Column object_type("object_type", DataTypes::eVarChar);     // data type
        Column object_parent("object_parent", DataTypes::eVarChar); // parent: table or null
        schemaTable->addColumn(object_name);
        schemaTable->addColumn(object_kind);
        schemaTable->addColumn(object_type);
        schemaTable->addColumn(object_parent);
        IStorageEngine *schemaStore = new BasicStorageEngine(path, true);
        Database*  impl_schema = reinterpret_cast<Database*>(schema.get()); 
        impl_schema->tables[__schema] = schemaTable;
        impl_schema->storageEngines[__schema] = schemaStore;
        
    }

    std::shared_ptr<IDatabase> IDatabase::newDatabase(const std::filesystem::path &path)
    {
        std::shared_ptr<IDatabase> db = nullptr;
        auto xdb = new Database(path);
        db.reset(xdb);
        xdb->_initSchemaDB();
        
        return db;
    }

    std::shared_ptr<IDatabase> IDatabase::openDatabase(const std::filesystem::path &path)
    {
        /*
        database folder structure:
        - folder name = database name
        - structure is stored into a table file (<db_name>.ru)
        - each table have it's data file (<table_name>.ru)
        */
        std::shared_ptr<IDatabase> ptr = nullptr;
        auto db = new Database(path);
        ptr.reset(db);
        db->_initSchemaDB();
        IStorageEngine *schemaStore = db->schema->getStorageEngine(__schema);
        TablePtr schemaTable =  db->schema->getTable(__schema);
        std::vector<Record> all_tables_structrue = schemaStore->SelectAll();

        for (auto it = all_tables_structrue.begin(); it != all_tables_structrue.end(); ++it)
        {
            // for simplicity purpose, we consider record of table objects are stored before column objects
            // The forthcoming implementation of "ORDER BY" will eliminate this limitation. 
            assert(it->fields_.size() % 4 == 0); // debugging
            RecordTablePtr rec = schemaTable->GetRecord(it->row_id_);
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
                db->newTable(xname);
            }
            else if (xkind == "COLUMN")
            {
                TablePtr tbl = db->getTable(xparent);
                Column cl(xname, getTypeFromString(xtype));
                tbl->addColumn(cl);
            }
            else
            {
                assert(false && "NOT IMPLEMENTED");
            }
        }
        return ptr;
    }

    TablePtr Database::newTable(const std::string& table_name)
    {
        if ( tables.find(table_name) != tables.end())
            return tables[table_name];
        
        TablePtr tbl ( new Table(table_name, this->shared_from_this())) ;
        tables[table_name]= tbl ;
        auto parent = path.parent_path();
        
        IStorageEngine* store = new BasicStorageEngine(parent.append(table_name + db_extension));
        storageEngines[table_name] = store;
        return tbl;
        
    }

    TablePtr Database::getTable(const std::string &tableName)
    {
        auto it = tables.find(tableName);
        if (it == tables.end())
            return nullptr;
        else
            return it->second;
    }

    std::vector<TablePtr> Database::getAllTables()
    {
        std::vector<TablePtr> res;
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

    IStorageEngine *Database::getStorageEngine(const std::string &tableName)
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
        auto tbl_schema = schema->getTable(__schema);
        assert(tbl_schema != nullptr);
        //drop storage 
        auto store_schema = schema->getStorageEngine(__schema);
        assert(store_schema != nullptr);
        store_schema->DropStorage();
        for (auto &&tbl : tables)
        {
            auto rec = tbl_schema->CreateRecord();
            rec->SetFieldValue("object_name", tbl.first);
            rec->SetFieldValue("object_kind", "TABLE");
            rec->SetFieldValue("object_type", "");
            rec->SetFieldValue("object_parent", "");
            rec->Save();
            const auto &cols = tbl.second->getColumns();
            for (const auto &it : cols)
            {
                auto rec = tbl_schema->CreateRecord();
                rec->SetFieldValue("object_name", it.getName());
                rec->SetFieldValue("object_kind", "COLUMN");
                rec->SetFieldValue("object_type", getStringFromType(it.getType()));
                rec->SetFieldValue("object_parent", tbl.first);
                rec->Save();
            }
        }

        return result;
    }

    Database::~Database()
    {   
        //before leaving must flush all data
        for (auto &&it : storageEngines)
             it.second->Flush();

        for (auto &&it : storageEngines)
            delete it.second;
    }
}
