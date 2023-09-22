#ifndef _H_RECORD__HH_
#define _H_RECORD__HH_

namespace ruru
{
    
    struct Field
    {
        Field();
        DataTypes type_;
        std::shared_ptr<char> value_;
        std::size_t GetHash() const;
        template <typename T>
        void SetValue(const T &value);
        void Reset();
        ~Field();
        const RecordLength_t GetSize() const;
    };

    struct Record
    {

        using SaveCallback_t = std::function<bool(Record&)>;
        RecordId row_id_;
        std::vector<Field> fields_;
        std::map<std::string, SaveCallback_t> callbacks_map_;
        const std::string GetKey() const;

        Record() : row_id_(-1){};
        Record(std::initializer_list<Field> init)
            : row_id_(-1), fields_{init} {}
        const RecordLength_t GetRowSize() const;

        //hard clone 
        void Clone(Record& ) const;
        bool RunCb();
        void AddCallback(const std::string& name,SaveCallback_t& cb );
    };
    
}

#endif //_H_RECORD__HH_