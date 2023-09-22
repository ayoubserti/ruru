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
    // Field

    Field::Field()
        :  type_(DataTypes::eUnknown), value_(nullptr)
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
        if ( value_ == nullptr)
            return 0;
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
                if( it.value_ != nullptr)
                    len += *(reinterpret_cast<uint64_t *>(it.value_.get()));
                break;
            }
            case DataTypes::eNull:
            break;

            default:
                throw new std::exception();
            }
        }
        return len;
    }

    void Record::AddCallback(const std::string& name,SaveCallback_t& cb )
    {
        callbacks_map_[name] = cb;
    }

    bool Record::RunCb()
    {
        for ( auto&& it : callbacks_map_)
        {
            if ( it.second(*this)== false)
            {
                return false;
            }
        }
        return true;
    }
}
