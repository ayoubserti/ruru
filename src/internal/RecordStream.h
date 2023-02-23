#pragma once
#include "ruru.h"
#include "record.h"
#include "basic_storage_engine.h"

namespace ruru::internal
{

    template <typename T>
    bool ReadField(T &stream, Field &rec)
    {
        // Read the 8-bit type field
        stream.read(reinterpret_cast<char *>(&rec.type_), sizeof(rec.type_));
        // Check for stream errors or end of file
        if (stream.fail())
        {
            return false;
        }

        switch (rec.type_)
        {
        case DataTypes::eNull:
            rec.value_ = nullptr;
            break;
        case DataTypes::eInteger:
        {
            rec.value_.reset((char *)malloc(sizeof(int64_t)));
            stream.read(rec.value_.get(), sizeof(uint64_t)); // TODO: manage endiness
        }
        break;

        case DataTypes::eDouble:
        {
            rec.value_.reset((char *)malloc(sizeof(double)));
            stream.read(rec.value_.get(), sizeof(double)); // TODO: manage endiness
        }
        break;
        case DataTypes::eVarChar:
        {
            // read the length
            uint64_t len;
            stream.read(reinterpret_cast<char *>(&len), sizeof(len));
            if (stream.fail())
                return false;

            rec.value_.reset((char *)malloc(len + sizeof(len)));
            memcpy(rec.value_.get(), &len, sizeof(len));
            stream.read(rec.value_.get() + sizeof(len), len);
            break;
        }
            // missing eBinary, until implementation of binary vector
        default:
            // !!< ERROR
            return false;
        }

        // Check for stream errors or end of file
        if (stream.fail())
        {
            return false;
        }
        return true;
    }

    template <typename T>
    void WriteField(T &stream, const Field &rec)
    {
        // Write the 8-bit type field

        if (rec.value_ == nullptr)
        {
            // NULL is a parent type. every types derive for it
            DataTypes v = DataTypes::eNull;
            stream.write(reinterpret_cast<const char *>(&v), sizeof(v));
            return;
        }
        stream.write(reinterpret_cast<const char *>(&rec.type_), sizeof(rec.type_));

        switch (rec.type_)
        {
        case DataTypes::eNull:
            break;
        case DataTypes::eInteger:
            stream.write(reinterpret_cast<const char *>(rec.value_.get()), sizeof(uint64_t));
            break;
        case DataTypes::eDouble:
            stream.write(reinterpret_cast<const char *>(rec.value_.get()), sizeof(double));
            break;
        case DataTypes::eVarChar:
        {
            // read the length
            uint64_t len = *(uint64_t *)(rec.value_.get());
            stream.write(reinterpret_cast<const char *>(rec.value_.get()), len + sizeof(len));
            break;
        }
        case DataTypes::eBinary:
        case DataTypes::eUnknown:
            throw new std::exception();
            // missing eBinary, until implementation of binary vector

            // default:
            //  !!< ERROR
        }
    }

    template <typename T>
    class RecordStream : public IRecordLoader
    {
    public:
        RecordStream(T &file_stream) : file_stream_(file_stream) {}

        // Read a record from the file.
        bool Read(RecordId &id, Record *record) override
        {
            bool result = true;
            if (id == -1)
            {
                // get the record in the current position
                file_stream_.read(reinterpret_cast<char *>(&record->row_id_), sizeof(record->row_id_));
                if (file_stream_.fail())
                    return false;

                // Read field list
                uint64_t field_nbr;
                file_stream_.read(reinterpret_cast<char *>(&field_nbr), sizeof(field_nbr));
                if (file_stream_.fail())
                    return false;

                record->fields_.resize(field_nbr);
                for (uint64_t i = 0; i < field_nbr; ++i)
                {
                    if (!ReadField(file_stream_, record->fields_[i]))
                    {
                        // error
                        return false;
                    }
                }
            }
            return true;
        }

        // Write a record to the file.
        bool Write(const Record &record) override
        {
            RecordId id = record.row_id_;
            uint64_t z = record.fields_.size();
            file_stream_.write(reinterpret_cast<const char *>(&id), sizeof(record.row_id_));
            file_stream_.write(reinterpret_cast<const char *>(&z), sizeof(uint64_t));
            for (auto &&it : record.fields_)
            {
                WriteField(file_stream_, it);
            }
            return true;
        }

        // Delete a record from the file.
        bool Delete(RecordId id) override
        {
            return false;
        }

    private:
        T &file_stream_;
    };

    using  RecordFile = RecordStream<std::fstream>;
}
