// Copyright (c) 2023 Ayoub Serti
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef _H_TABLE__H_
#define _H_TABLE__H_


namespace ruru
{

    class RecordTable;
    class Record;
    class Database;
    class ResultSet;
    class Filter;

    enum class RecordType
    {
        eNew = 1,
        eModifyed,

        eUnknown
    };

    class Column
    {
        std::string name;
        DataTypes type;
        bool isPrimaryKey;
        bool isNullable;
        bool isAutoIncrement;

    public:
        /*
         * Constructor
         *
         * @param name name of the column
         * @param type type of the column (eInteger, eDouble, eVarChar, eBinary)
         * @param isPrimaryKey define if the column is primary key or not
         * @param isNullable define if the column can store null value or not
         * @param isAutoIncrement define if the column has auto increment feature
         */
        Column(std::string name, DataTypes type, bool isPrimaryKey = false, bool isNullable = true, bool isAutoIncrement = false);

        /**
         * get the name of the column
         * @return the name of the column
         */
        const std::string &getName() const;

        /**
         * get the type of the column
         * @return the type of the column
         */
        DataTypes getType() const;

        /**
         * check if the column is primary key
         * @return true if it is primary key, false otherwise
         */
        bool isPrimary() const;

        /**
         * check if the column is auto increment
         * @return true if it is auto increment, false otherwise
         */
        bool isAutoInc() const;

        /**
         * check if the column is not nullable
         * @return true if it is not nullable, false otherwise
         */
        bool isNotNull() const;
    };

    class Table
    {
        Database* database;
        std::string name;
        std::vector<Column> columns;
        std::unordered_map<std::string, int> columns_name_to_index;
        std::map<std::pair<std::string, std::string>, int> indices;
        
        //friend class
        //friend class ResultSet;

        //private member function
        RecordTable* _CreateRecordTableFromRec(  Record* rec);

    public:
        Table(std::string name, Database* db);

        // Adding a new column to table
        void addColumn(const Column &col);

        // Getting column index by name
        int getColumnIndex(const std::string &col_name) const;

        // Returning column by index
        Column &getColumn(int index);

        // Returning column by name
        Column &getColumn(const std::string &col_name);

        // Returning table name
        const std::string &getName() const;

        // Adding Index to table
        void addIndex(const std::string &col_name, const std::string &index_name);

        // Getting index by name
        int getIndex(const std::string &index_name) const;

        // Removing index by name
        void removeIndex(const std::string &index_name);

        //create a Record from table
        RecordTable* CreateRecord();

        const std::vector<Column>& getColumns() const {
            return columns;
        }

        Database* getDatabase() { return database;}

        // Lookup a result set by filter
        // for instance, get all records for which 'age' > 34
        ResultSet* Search(const std::vector<std::shared_ptr<Filter> >& filters);


        //get record from storage
        RecordTable*  GetRecord( RecordId id);


    };

    // RecordTable represent a record inside the table
    // it encapsulates the Record 
    class RecordTable
    {
        Table*  table;
        Record* record;
        RecordType type;
        RecordTable ( Table* tbl , Record* rec);
        friend class Table;
        
        public:
        
        RecordTable() = delete;

        void SetFieldValue (  const std::string& field_name, int64_t value); 

        void SetFieldValue (  const std::string& field_name, double value); 
        
        void SetFieldValue (  const std::string& field_name, const std::string& value); 
        
        void SetFieldValue (  const std::string& field_name, const std::shared_ptr<char>& value); 

        bool 
        GetFieldValue( const std::string& field_name, int64_t& value);

        bool
        GetFieldValue( const std::string& field_name, double& value);

        bool
        GetFieldValue( const std::string& field_name, std::string& value);

        // binary 
        bool
        GetFieldValue( const std::string& field_name, std::shared_ptr<char>& value);


        //Save Record
        bool    Save();

    };

}

#endif //_H_TABLE__H_