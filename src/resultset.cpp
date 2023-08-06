// Copyright (c) 2023 Ayoub Serti
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "pch.h"
#include "ruru.h"

namespace ruru
{
    ResultSet::ResultSet(const Filters_t &filters)
        : filters_(filters), iter_(-1)
    {
    }

    std::shared_ptr<RecordTable> ResultSet::First()
    {
        RecordTablePtr rec(nullptr);
        iter_ = 0;
        if (!Eof())
        {
            rec = table_->GetRecord(records_id_[iter_]) ;
        }
        return rec;
    }

    bool ResultSet::Eof()
    {
        if (iter_ >= (int64_t)records_id_.size())
            return true;
        return false;
    }

    std::shared_ptr<RecordTable> ResultSet::Next()
    {
        RecordTablePtr rec(nullptr);
        iter_++;
        if (!Eof())
        {
            rec  = table_->GetRecord(records_id_[iter_]) ;
        }
        return rec;
    }

    int64_t         ResultSet::GetSize()
    {
        return records_id_.size();
    }

}