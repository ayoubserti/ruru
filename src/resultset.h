// Copyright (c) 2023 Ayoub Serti
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef _H_RESULTSET_HH__
#define _H_RESULTSET_HH__

#include <variant>

namespace ruru
{
    class Table;
    class Column;
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

    using Value_t = std::variant<int64_t, double, std::string>;

    class Filter;
    class Table;
    class RecordTable;

    using Filters_t = std::vector<std::shared_ptr<Filter>>;

    class ResultSet
    {
        Table *table_;
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
    };

    struct Filter
    {
        Table *table;
        Column *column;
        OperatorType oper;
        Value_t value1;
        Value_t value2;
        uint16_t column_indx;

        Filter(Table *table, Column *col, OperatorType oper, const Value_t &v1, const Value_t &v2);

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
}

#endif