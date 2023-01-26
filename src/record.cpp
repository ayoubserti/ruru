// Copyright (c) 2023 Ayoub Serti
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "pch.h"
#include "ruru.h"
#include "record.h"
#include "field_impl.hpp"

namespace ruru
{

    template <typename T>
    bool Read(T &stream, Field &rec)
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
        case DataTypes::eInteger:
        {
            rec.value_.reset((char *)malloc( sizeof(int64_t)));
            stream.read(rec.value_.get(), sizeof(uint64_t)); //TODO: manage endiness
        }
            break;

        case DataTypes::eDouble:
            {
            rec.value_.reset((char *)malloc( sizeof(double)));
            stream.read(rec.value_.get(), sizeof(double)); //TODO: manage endiness
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
    void Write(T &stream, const Field &rec)
    {
        // Write the 8-bit type field
        stream.write(reinterpret_cast<const char *>(&rec.type_), sizeof(rec.type_));
        switch (rec.type_)
        {
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
            stream.write(reinterpret_cast<const char *>(rec.value_.get()), len +sizeof(len) );
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

    // Field

    Field::Field()
    :name_("")
    ,type_(DataTypes::eUnknown)
    ,value_(nullptr)
    {

    }

    void Field::Reset()
    {
        switch (type_)
        {
        case DataTypes::eInteger:
            SetValue(0);
            break;
        case DataTypes::eDouble:
            SetValue(0.0f);
            break;
        case DataTypes::eVarChar:
            SetValue("");
            // missing eBinary, until implementation of binary vector
            break;
        case DataTypes::eBinary:
        case DataTypes::eUnknown:
        default:
            break;
            // throw new std::exception();
        }
    }

    std::size_t Field::GetHash() const
    {
        switch (type_)
        {
        case DataTypes::eInteger:
            return std::hash<uint64_t>{}(reinterpret_cast<uint64_t>(value_.get()));

        case DataTypes::eDouble:
            return std::hash<double>{}(*reinterpret_cast<double *>(value_.get()));

        case DataTypes::eVarChar:
            return std::hash<std::string>{}(std::string(value_.get()));
            // missing eBinary, until implementation of binary vector

        default:
            // !!< ERROR
            return 0;
        }
    }
    Field::~Field()
    {
    }
    // Record

    const std::string Record::GetKey() const
    {
        // calculate a key
        std::size_t seed = fields_.size();
        for (const auto &v : fields_)
        {
            seed ^= std::hash<size_t>{}(v.GetHash()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return std::to_string(seed);
    }

    const RecordLength_t Record::GetRowSize() const
    {
        RecordLength_t len = 8 + 8; // sizeof(row_id_) + sizeof ( nbr_field);
        for (auto &&it : fields_)
        {
            len += sizeof(it.type_);
            switch (it.type_)
            {
            case DataTypes::eInteger:
                len += sizeof(int64_t);
                break;
            case DataTypes::eDouble:
                len += sizeof(double);
                break;
            case DataTypes::eVarChar:
            case DataTypes::eBinary:
            {
                len += sizeof(uint64_t);
                len += *(reinterpret_cast<uint64_t *>(it.value_.get()));
                break;
            }

            default:
                throw new std::exception();
            }
        }
        return len;
    }
    // RecordFile

    RecordFile::RecordFile(std::fstream &inFile)
        : file_stream_(inFile) {}

    bool RecordFile::Read(RecordId &id, Record *record)
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
                if (!ruru::Read(file_stream_, record->fields_[i]))
                {
                    // error
                    return false;
                }
            }
        }

        return true;
    }

    bool RecordFile::Write(const Record &record)
    {
        RecordId id = record.row_id_;
        uint64_t z = record.fields_.size();
        file_stream_.write(reinterpret_cast<const char *>(&id), sizeof(record.row_id_));
        file_stream_.write(reinterpret_cast<const char *>(&z), sizeof(uint64_t));
        for (auto &&it : record.fields_)
        {
            ruru::Write(file_stream_, it);
        }
        return true;
    }

    bool RecordFile::Delete(RecordId id)
    {
        return false;
    }
}
