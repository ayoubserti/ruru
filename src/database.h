#ifndef _H_DATABASE_HH__
#define _H_DATABASE_HH__

namespace ruru
{
    class Table;
    class StorageEngine;

    class Database
    {
        std::string name;
        std::map<std::string, Table *> tables;
        std::map<std::string, StorageEngine *> storageEngines;
        std::unique_ptr<Database>       schema; //{nullptr};

        Database(const std::string &name);

        void _initSchemaDB();
    public:
        
        static Database* newDatabase(const std::string &name);
        
        //openDatabase from path
        static Database* openDatabase(const std::filesystem::path& path);

        // Adding a new table to the database
        void addTable(Table *table, StorageEngine *storageEngine);

        // Getting a table by name
        Table *getTable(const std::string &tableName);

        // Getting all tables in the database
        std::vector<Table *> getAllTables();

        // Removing a table from the database
        void removeTable(const std::string &tableName);

        // Getting the storage engine for a table
        StorageEngine *getStorageEngine(const std::string &tableName);

        //Save Database schema
        bool   saveSchema(const std::filesystem::path& path);

        ~Database();
        
    };

}

#endif //_H_DATABASE_HH__