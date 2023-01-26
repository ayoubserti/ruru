// Copyright (c) 2023 Ayoub Serti
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "pch.h"
#include "ruru.h"
#include "table.h"
#include "resultset.h"
#include "record.h"
#include "database.h"

namespace ruru
{
    ResultSet::ResultSet(const Filters_t &filters)
        : filters_(filters), iter_(0)
    {
    }

    std::shared_ptr<RecordTable> ResultSet::First()
    {
        std::shared_ptr<RecordTable> rec(nullptr);
        iter_ = 0;
        if (!Eof())
        {
            rec.reset(table_->GetRecord(records_id_[iter_]));
        }
        return rec;
    }

    bool ResultSet::Eof()
    {
        if (iter_ >= records_id_.size())
            return true;
        return false;
    }

    std::shared_ptr<RecordTable> ResultSet::Next()
    {
        std::shared_ptr<RecordTable> rec(nullptr);
        iter_++;
        if (!Eof())
        {
            rec.reset(table_->GetRecord(records_id_[iter_]));
        }
        return rec;
    }

    // Filter
    Filter::Filter(Table *tbl, Column *col, OperatorType oper, const Value_t &v1, const Value_t &v2)
        : table(tbl), column(col), oper(oper), value1(v1), value2(v2)
    {
        column_indx = tbl->getColumnIndex(col->getName());
    }

}