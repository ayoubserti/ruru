#ifndef _H_DATABASE_HH__
#define _H_DATABASE_HH__

#include "ruru.h"
namespace ruru
{
    //forward declaration
    class IStorageEngine;
    class Table;
    
    //Database class is an implementation of interface IDatabase
    class Database : public IDatabase
    {
        std::string name;
        std::filesystem::path path;
        std::map<std::string, TablePtr> tables;
        std::map<std::string, IStorageEngine *> storageEngines;
        std::shared_ptr<Database> schema; //{nullptr};
        IStorageEngineFactory*  storeFactory;
        
        Database(const std::filesystem::path &path);

        void _initSchemaDB();

        friend class IDatabase;

    public:

        // Adding a new table to the database
        TablePtr newTable(const std::string &table_name) override;

        // Getting a table by name
        TablePtr getTable(const std::string &tableName) override;

        // Getting all tables in the database
        std::vector<TablePtr> getAllTables() override;

        // Removing a table from the database
        void removeTable(const std::string &tableName) override;

        // Getting the storage engine for a table
        IStorageEngine *getStorageEngine(const std::string &tableName) ;

        // Save Database schema
        bool saveSchema(const std::filesystem::path &path) override;

        // set The storage Engine Factory
        bool setStorageEngineFactory ( IStorageEngineFactory* factory) override; 

        virtual ~Database();
    };

}

#endif //_H_DATABASE_HH__