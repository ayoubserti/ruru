// Copyright (c) 2023 Ayoub Serti
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "pch.h"
#include "ruru.h"
#include "table.h"
#include "record.h"
#include "database.h"
#include "storage_engine.h"
#include "resultset.h"

namespace ruru
{

    Column::Column(std::string name, DataTypes type, bool isPrimaryKey, bool isNullable, bool isAutoIncrement) : name(std::move(name)), type(type), isPrimaryKey(isPrimaryKey), isNullable(isNullable), isAutoIncrement(isAutoIncrement)
    {
    }

    const std::string &Column::getName() const
    {
        return name;
    }

    DataTypes Column::getType() const
    {
        return type;
    }

    bool Column::isPrimary() const
    {
        return isPrimaryKey;
    }

    bool Column::isAutoInc() const
    {
        return isAutoIncrement;
    }

    bool Column::isNotNull() const
    {
        return !isNullable;
    }

    Table::Table(std::string name, Database *db) : name(std::move(name)), database(db) {}

    void Table::addColumn(const Column &col)
    {
        columns.push_back(col);
        columns_name_to_index[col.getName()] = columns.size() - 1;
    }
    // Getting column index by name
    int
    Table::getColumnIndex(const std::string &col_name) const
    {
        auto it = columns_name_to_index.find(col_name);
        if (it == columns_name_to_index.end())
            return -1;
        else
            return it->second;
    }

    // Returning column by index
    Column &Table::getColumn(int index)
    {
        return columns[index];
    }

    // Returning column by name
    Column &Table::getColumn(const std::string &col_name)
    {
        auto index = getColumnIndex(col_name);
        return columns[index];
    }

    // Returning table name
    const std::string &Table::getName() const
    {
        return name;
    }

    // Adding Index to table
    void Table::addIndex(const std::string &col_name, const std::string &index_name)
    {
        auto index = getColumnIndex(col_name);
        indices[std::make_pair(index_name, col_name)] = index;
    }

    // Getting index by name
    int Table::getIndex(const std::string &index_name) const
    {
        auto it = indices.find(std::make_pair(index_name, index_name));
        if (it == indices.end())
            return -1;
        else
            return it->second;
    }

    // Removing index by name
    void Table::removeIndex(const std::string &index_name)
    {
        indices.erase(std::make_pair(index_name, index_name));
    }

    RecordTable *Table::CreateRecord()
    {
        Record *rec = new Record();
        // prepare fields
        for (auto &&it : columns)
        {
            Field fl;
            fl.Reset();
            fl.name_ = it.getName();
            fl.type_ = it.getType();
            rec->fields_.push_back(fl);
        }
        RecordTable *rectbl = new RecordTable(this, rec);
        rectbl->type = RecordType::eNew;
        return rectbl;
    }

    RecordTable *Table::_CreateRecordTableFromRec(Record *rec)
    {
        RecordTable *rectbl = new RecordTable(this, rec);
        rectbl->type = RecordType::eModifyed;
        return rectbl;
    }

    ResultSet *Table::Search(const Filters_t &filters)
    {
        ResultSet *result = new ResultSet(filters);
        result->table_ = this;

        // apply the search in the StorageEngine and retrieve list of record Id
        StorageEngine *store = database->getStorageEngine(getName());
        auto rows = store->Lookup(filters);
        result->records_id_ = rows;
        return result;
    }

    RecordTable *Table::GetRecord(RecordId id)
    {
        // id is internal ID ( rowid)
        RecordTable *rectable = nullptr;
        StorageEngine *store = getDatabase()->getStorageEngine(getName());
        Record *rec = store->LoadRecord(id);
        if (rec != nullptr)
        {
            rectable = _CreateRecordTableFromRec(rec);
        }
        return rectable;
    }

    // RecordTable

    RecordTable::RecordTable(Table *tbl, Record *rec)
        : table(tbl), record(rec)
    {
    }

    void RecordTable::SetFieldValue(const std::string &field_name, int64_t value)
    {
        int i = table->getColumnIndex(field_name);
        if (i != -1)
        {
            record->fields_[i].SetValue(value);
        }
    }

    void RecordTable::SetFieldValue(const std::string &field_name, double value)
    {
        int i = table->getColumnIndex(field_name);
        if (i != -1)
        {
            record->fields_[i].SetValue(value);
        }
    }
    void RecordTable::SetFieldValue(const std::string &field_name, const std::string &value)
    {
        int i = table->getColumnIndex(field_name);
        if (i != -1)
        {
            record->fields_[i].SetValue(value);
        }
    }

    void RecordTable::SetFieldValue(const std::string &field_name, const std::shared_ptr<char> &value)
    {
        /* int i = table->getColumnIndex(field_name);
         if (i != -1)
         {
             record->fields_[i].SetValue(value);
         }*/
    }

    bool RecordTable::GetFieldValue(const std::string &field_name, int64_t &value)
    {
        int i = table->getColumnIndex(field_name);
        if (i == -1)
        {
            return false;
        }
        Column cl = table->getColumn(field_name);
        if (cl.getType() != DataTypes::eInteger)
            return false;
        Field fl = record->fields_[i];
        UnionValues v;
        v.txt = fl.value_.get();
        value = v.nbr;
        return true;
    }

    bool RecordTable::GetFieldValue(const std::string &field_name, double &value)
    {
        int i = table->getColumnIndex(field_name);
        if (i == -1)
        {
            return false;
        }
        Column cl = table->getColumn(field_name);
        if (cl.getType() != DataTypes::eDouble)
            return false;
        Field fl = record->fields_[i];
        UnionValues v;
        v.txt = fl.value_.get();
        value = v.floating;

        return true;
    }

    bool RecordTable::GetFieldValue(const std::string &field_name, std::string &value)
    {
        int i = table->getColumnIndex(field_name);
        if (i == -1)
        {
            return false;
        }
        Column cl = table->getColumn(field_name);
        if (cl.getType() != DataTypes::eVarChar)
            return false;
        Field fl = record->fields_[i];

        uint64_t *len = reinterpret_cast<uint64_t *>(fl.value_.get());
        if (*len == 0)
            value = "";
        else
            value.assign(fl.value_.get() + 8, *len);

        return true;
    }

    bool RecordTable::GetFieldValue(const std::string &field_name, std::shared_ptr<char> &value)
    {
        int i = table->getColumnIndex(field_name);
        if (i == -1)
        {
            return false;
        }
        Column cl = table->getColumn(field_name);
        if (cl.getType() != DataTypes::eBinary)
            return false;
        Field fl = record->fields_[i];
        value = fl.value_;
        return true;
    }

    bool RecordTable::Save()
    {
        StorageEngine *store = table->getDatabase()->getStorageEngine(table->getName());
        return store->Save(*record, type == RecordType::eNew);
    }
}