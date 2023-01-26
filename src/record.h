#ifndef _H_RECORD__HH_
#define _H_RECORD__HH_
#include "ruru.h"

namespace ruru
{
    struct Field
    {
        Field();
        std::string name_;
        DataTypes type_;
        std::shared_ptr<char> value_;
        std::size_t GetHash() const;
        template <typename T> void SetValue( const T& value);
        void Reset();
        ~Field();
        const RecordLength_t GetSize() const;
    };

    struct Record
    {
        RecordId row_id_;
        std::vector<Field> fields_;
        const std::string GetKey() const;

        Record(): row_id_(-1) {};
        Record(std::initializer_list<Field> init)
        :row_id_(-1)
        ,fields_{init}{}
        const RecordLength_t  GetRowSize() const;
        
    };

    class RecordFile
    {
    public:
        RecordFile(std::fstream &file_stream);

        // Read a record from the file.
        bool Read(RecordId &id, Record *record);

        // Write a record to the file.
        bool Write(const Record &record);

        // Delete a record from the file.
        bool Delete(RecordId id);

    private:
        std::fstream &file_stream_;
    };

}

#endif //_H_RECORD__HH_