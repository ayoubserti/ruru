#ifndef __H_RURU_HH__
#define __H_RURU_HH__

namespace ruru
{

    //global function
    /*
        @fn     Init
        @def    Initialize rurudb
                Load & register StorageEngine Factories
    */
    void Init();

    // define the record ID type
    typedef uint64_t RecordId;

    // defines
    typedef int64_t RecordLength_t;
    typedef int64_t RecordPosition_t;

    // List of DataTypes
    enum class DataTypes : uint8_t
    {
        eUnknown = 0,
        eInteger = 1,
        eDouble,
        eVarChar,
        eBinary,

        eNull = 0xFF
    };

    enum class RecordType
    {
        eNew = 1,
        eModifyed,
        eUnknown = 0
    };

     enum class OperatorType : uint8_t
    {
        eEqual = 1,
        eGreater,
        eGreaterOrEq,
        eLesser,
        eLesserOrEq,
        eInRange,
        eOutRange,

        eUnknown = 0
    };


    static const char* db_extension = ".ru";
    static const char* _basic_factory = "_basic_factory";

    //forward class
    class Record;
    class Filter;
    class Table;
    class RecordTable;
    class ResultSet;
    class IDatabase;

    using TablePtr = std::shared_ptr<Table>;
    using RecordTablePtr = std::shared_ptr<RecordTable>;
    using Filters_t = std::vector<std::shared_ptr<Filter> >;
    using ResultSetPtr=  std::shared_ptr<ResultSet>;
    using DatabasePtr =  std::shared_ptr<IDatabase>;
    using Value_t = std::variant<int64_t, double, std::string>;
    using Filters_t = std::vector<std::shared_ptr<Filter>>;

    //APIs
    //interface StorgeEngine
    class IStorageEngine
    {
    public:
        // Insert a record into the table
        virtual void Insert(const Record &record) = 0;

        // Select all records from the table
        virtual std::vector<Record> SelectAll() = 0;

        // Look up a record by key
        virtual std::vector<Record> Lookup(const std::string &key) = 0;

        // Look up records by filter
        virtual std::vector<RecordId> Lookup(const Filters_t &filters) = 0;

        // LoadRecord
        virtual Record *LoadRecord(RecordId id) = 0;

        // Save a record and set a record id
        virtual bool Save(Record &record, bool isNew) = 0;

        // Flush
        virtual bool Flush() = 0;

        // Drop Storage
        virtual bool DropStorage() = 0;

        virtual ~IStorageEngine(){};
    };
    //interface StorageEngineFactory
    class IStorageEngineFactory 
    {
        public:
        virtual IStorageEngine* createStorageEngine( const std::string& name) = 0;
        virtual std::string getName(  ) = 0 ;
    };
    //Interface IDatabase
    class IDatabase  : public std::enable_shared_from_this<IDatabase>
    {

    public:
        static std::shared_ptr<IDatabase> newDatabase(const std::filesystem::path &path);

        // openDatabase from path
        static std::shared_ptr<IDatabase> openDatabase(const std::filesystem::path &path);

        // Adding a new table to the database
        virtual TablePtr newTable(const std::string &table_name) = 0;

        // Getting a table by name
        virtual TablePtr getTable(const std::string &tableName) = 0;

        // Getting all tables in the database
        virtual std::vector<TablePtr> getAllTables() = 0;

        // Removing a table from the database
        virtual void removeTable(const std::string &tableName) = 0;

        // Save Database schema
        virtual bool saveSchema(const std::filesystem::path &path) = 0;

        // set The storage Engine Factory
        /*
            \param factory the StorageEngineFactory to create Storage at need
            return true the first call, the keep returing false
            must be called one time
            The caller owns the pointer.  
        */
        virtual bool setStorageEngineFactory ( IStorageEngineFactory* factory) = 0;

        virtual ~IDatabase() {};

    };

    
  class ResultSet
    {
        Table* table_;
        Filters_t filters_;
        std::vector<RecordId> records_id_;
        int64_t iter_;
        ResultSet(const Filters_t &filters);
        friend class Table;

    public:
        // get the first record
        std::shared_ptr<RecordTable> First();
        // is the end of the result set reached
        bool Eof();
        // move to the next record
        std::shared_ptr<RecordTable> Next();
        // get size
        int64_t     GetSize();
    };

    struct Filter
    {
        uint16_t column_indx;
        OperatorType oper;
        Value_t value1;
        Value_t value2;

        Filter(uint16_t column_indx, OperatorType oper, const Value_t &v1, const Value_t &v2)
        :column_indx(column_indx)
        ,oper(oper)
        ,value1(v1)
        ,value2(v2)
        {}

        template <typename T>
        bool Apply(const T &value) const
        {
            if (oper == OperatorType::eEqual)
            {
                Value_t v = value;
                return value1 == v;
            }
            else if (oper == OperatorType::eGreater)
            {
                Value_t v = value;
                return v > value1;
            }
            else if (oper == OperatorType::eGreaterOrEq)
            {
                Value_t v = value;
                return v >= value1;
            }
            else if (oper == OperatorType::eLesser)
            {
                Value_t v = value;
                return v < value1;
            }
            else if (oper == OperatorType::eLesserOrEq)
            {
                Value_t v = value;
                return v <= value1;
            }
            else
            {
                throw new std::exception();
                return false;
            }
            return false;
        }
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
        std::weak_ptr<IDatabase> database;
        std::string name;
        std::vector<Column> columns;
        std::unordered_map<std::string, int> columns_name_to_index;
        std::map<std::pair<std::string, std::string>, int> indices;

        // friend class
        friend class Database;

        // private member function
        RecordTablePtr _CreateRecordTableFromRec(Record *rec);

        Table(std::string name, std::shared_ptr<IDatabase> db);

    public:
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

        // create a Record from table
        RecordTablePtr CreateRecord();

        const std::vector<Column> &getColumns() const
        {
            return columns;
        }

        std::weak_ptr<IDatabase> getDatabase() { return database; }

        // Lookup a result set by filter
        // for instance, get all records for which 'age' > 34
        ResultSetPtr Search(const std::vector<std::shared_ptr<Filter>> &filters);

        // get record from storage
        RecordTablePtr GetRecord(RecordId id);
    };

    // RecordTable represent a record inside the table
    // it encapsulates the Record
    class RecordTable
    {
        Table *table;
        Record *record;
        RecordType type;
        RecordTable(Table *tbl, Record *rec);
        friend class Table;

    public:
        RecordTable() = delete;

        void SetFieldNull(const std::string &field_name);
        void SetFieldValue(const std::string &field_name, int64_t value);

        void SetFieldValue(const std::string &field_name, double value);

        void SetFieldValue(const std::string &field_name, const std::string &value);

        void SetFieldValue(const std::string &field_name, const std::shared_ptr<char> &value);

        bool 
        IsFieldNull(const std::string &field_name , bool& value );

        bool
        GetFieldValue(const std::string &field_name, int64_t &value);

        bool
        GetFieldValue(const std::string &field_name, double &value);

        bool
        GetFieldValue(const std::string &field_name, std::string &value);

        // binary
        bool
        GetFieldValue(const std::string &field_name, std::shared_ptr<char> &value);

        // Save Record
        bool Save();

        // dtor
        virtual ~RecordTable();
    };



    bool registerEngineFactory( const std::string& name , IStorageEngineFactory* engineFactory);

    IStorageEngineFactory* getEngineFactory( const std::string& name);


    //tools
    DataTypes getTypeFromString(const std::string &name);
    std::string getStringFromType(DataTypes type);

}

#endif //__H_RURU_HH__