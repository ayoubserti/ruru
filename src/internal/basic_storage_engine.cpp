// Copyright (c) 2023 Ayoub Serti
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "pch.h"
#include "ruru.h"
#include "basic_storage_engine.h"
#include "record.h"

using namespace ruru;
using namespace ruru::internal;

// tools
bool _ApplyFilter(const Record &rec, const Filter &filter)
{
    // quick & dirty solution
    //  we may consider to create operator functions
    bool result = true;
    Field fl = rec.fields_[filter.column_indx];
    switch (fl.type_)
    {
    case DataTypes::eInteger:
    {
        int64_t value = *reinterpret_cast<int64_t *>(fl.value_.get());
        return filter.Apply(value);
    }
    break;
    case DataTypes::eDouble:
    {
        double value = *reinterpret_cast<double *>(fl.value_.get());
        return filter.Apply(value);
    }

    break;
    case DataTypes::eVarChar:
    {
        std::string value = fl.value_.get();
        return filter.Apply(value);
    }

    break;
    case DataTypes::eNull:
    //TODO implement (is null, is not null ) filters
    break;
    default:
        throw new std::exception();
    }
    return result;
}


IStorageEngine* BasicStorageEngineFactory::createStorageEngine( const std::string& file_name)
{
    return new BasicStorageEngine(file_name);
}

std::string  BasicStorageEngineFactory::getName( )
{
    return _basic_factory;
}

// Constructor
BasicStorageEngine::BasicStorageEngine(const std::string &file_name, bool forSchema)
    : file_name_(file_name),
      current_rec_id_(-1),
      is_for_schema_(forSchema)
{
    if (!is_for_schema_)
    {
        // Load the index from the index file
        LoadIndex();
        // Load row_id index
        LoadHiddenIndex();
    }
}

// Insert a record into the table
void BasicStorageEngine::Insert(const Record &record)
{
    // Open the file in append mode
    std::fstream file(file_name_, std::ios::app);

    if (!is_for_schema_)
    {
        // Update the index
        index_.Insert(record.GetKey(), file.tellp());

        // update hidden index
        row_id_index_.Insert(record.row_id_, std::make_pair<RecordLength_t, RecordPosition_t>(record.GetRowSize(), file.tellp()));
    }

    RecordFile record_file(file);
    record_file.Write(record);

    // Close the file
    file.close();
}

// Select all records from the table
std::vector<Record> BasicStorageEngine::SelectAll()
{
    // Open the file in read mode
    std::fstream file(file_name_);

    std::vector<Record> records;
    if (!file.is_open())
        return records;

    //!!<< Implement a Cache system
    while (!file.eof())
    {
        // read record from
        RecordFile rec_file(file);
        Record rec;
        RecordId rec_id = -1;
        if (rec_file.Read(rec_id, &rec))
            records.push_back(rec);
    }

    // Close the file
    file.close();

    return records;
}

// Look up a record by key
std::vector<Record> BasicStorageEngine::Lookup(const std::string &key)
{
    if (is_for_schema_)
        return {};
    // Use the index to find the offset of the record in the file
    int offset = index_.Lookup(key);

    // Open the file in read mode
    std::fstream file(file_name_);

    // Seek to the offset of the record
    file.seekg(offset);

    std::vector<Record> values;
    RecordFile f(file);
    Record v;
    RecordId id = -1;
    f.Read(id, &v);
    values.push_back(v);

    // Close the file
    file.close();

    return values;
}

std::vector<RecordId> BasicStorageEngine::Lookup(const Filters_t &filters)
{

    std::vector<RecordId> rowsid;

    // table full scan
    auto entries = row_id_index_.GetEntries();
    for (auto &it : entries)
    {
        Record rec;
        if (_LoadRecord(it.second.second, rec) > 0)
        {
            // apply filters
            bool ok = true;
            for (auto &&filter : filters)
            {
                if (!_ApplyFilter(rec, *filter.get()))
                {
                    ok = false;
                    break;
                }
            }
            if (ok)
                rowsid.push_back(it.first);
        }
    }

    return rowsid;
}

Record *BasicStorageEngine::LoadRecord(RecordId id)
{
    if (!is_for_schema_)
    {
        if (!row_id_index_.Exists(id))
            return nullptr;
        auto entry = row_id_index_.Lookup(id);
        Record *rec = new Record();
        _LoadRecord(entry.second, *rec);
        return rec;
    }
    else
    {
        // schema file is a small file, quick scan
        std::fstream file(file_name_);
        if (!file.is_open())
            return nullptr;
        Record *rec = new Record();
        while (!file.eof())
        {
            RecordFile recfile(file);
            RecordId zid = -1;
            if (recfile.Read(zid, rec) && rec->row_id_ == id)
                return rec;
        }
        delete rec;

        return nullptr;
    }
}

// Load the index from the index file
void BasicStorageEngine::LoadIndex()
{
    // Open the index file in read mode
    std::ifstream index_file(file_name_ + ".index");
    std::string line;
    while (std::getline(index_file, line))
    {
        // Split the line by ',' to get the key and file offset
        size_t pos = line.find(',');
        std::string key = line.substr(0, pos);
        int offset = std::stoi(line.substr(pos + 1));
        index_.Insert(key, offset);
    }
    index_file.close();
}
// Load the index from the index file
void BasicStorageEngine::LoadHiddenIndex()
{
    // Open the index file in read mode
    std::ifstream index_file(file_name_ + ".row.index"); // keylenghpos
    if (!index_file.is_open())
        return;
    while (!index_file.eof())
    {
        RecordId key;
        RecordLength_t rec_len;
        RecordPosition_t rec_pos;
        index_file.read(reinterpret_cast<char *>(&key), sizeof(RecordId));
        index_file.read(reinterpret_cast<char *>(&rec_len), sizeof(RecordLength_t));
        index_file.read(reinterpret_cast<char *>(&rec_pos), sizeof(RecordPosition_t));
        row_id_index_.Insert(key, std::make_pair(rec_len, rec_pos));
    }
    index_file.close();

    if (row_id_index_.GetSize())
        current_rec_id_ = row_id_index_.GetMax();
}

// Save the index to the index file
void BasicStorageEngine::SaveIndex()
{
    // Open the index file in write mode
    {
        std::ofstream index_file(file_name_ + ".index", std::ios::trunc);

        // Iterate over the index and write the key-offset pairs to the file
        std::vector<std::pair<std::string, int>> entries = index_.GetEntries();
        for (const auto &entry : entries)
        {
            index_file << entry.first << "," << entry.second << "\n";
        }

        // Close the index file
        index_file.close();
    }
    // Open the index file in write mode
    {
        std::ofstream index_file(file_name_ + ".row.index", std::ios::trunc);
        if (!index_file.is_open())
            return;
        // Iterate over the index and write the key-offset pairs to the file
        auto entries = row_id_index_.GetEntries();
        for (const auto &entry : entries)
        {
            index_file.write(reinterpret_cast<const char *>(&entry.first), sizeof(RecordId));
            index_file.write(reinterpret_cast<const char *>(&entry.second.first), sizeof(RecordLength_t));
            index_file.write(reinterpret_cast<const char *>(&entry.second.second), sizeof(RecordPosition_t));
        }

        // Close the index file
        index_file.close();
    }
}

// Save the record into storage
bool BasicStorageEngine::Save(Record &record, bool isNew)
{
    if (isNew)
    {
        current_rec_id_++;
        record.row_id_ = current_rec_id_;
        Insert(record);
        return true;
    }
    else
    {
        // the record isn't a new one
        // get the position inside the hidden index
        // if the old version have  size greater or equal to the new size --> write on the same position
        // else modify the hidden index and put at the end
        // it worths to notice this implementation still need to handle "holes" generated by this mecanism
        auto info = row_id_index_.Lookup(record.row_id_);
        RecordLength_t size = record.GetRowSize();
        if (info.first >= size)
        {
            // update in the same position
            std::fstream file(file_name_);
            if (file.is_open())
            {
                return false;
            }
            file.seekp(info.second);
            RecordFile rec_file(file);
            if (!rec_file.Write(record))
                return false;
            info.first = size;
            row_id_index_.Insert(record.row_id_, info);
            file.close();
        }
        else
        {
            // insert it at the end
            Insert(record);
            return true;
        }
    }
    return false;
}

bool BasicStorageEngine::Flush()
{
    if (!is_for_schema_)
    {
        SaveIndex();
    }
    return true;
}

bool BasicStorageEngine::DropStorage()
{
    return std::filesystem::remove(file_name_);
}

RecordLength_t BasicStorageEngine::_LoadRecord(RecordPosition_t position, Record &rec)
{
    RecordLength_t len = 0;
    std::fstream file(file_name_);

    RecordFile recInFile(file);
    RecordId id = -1;
    file.seekg(position);
    if (recInFile.Read(id, &rec))
    {
        len = file.tellg() - position;
    }
    file.close();
    return len;
}

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
    case DataTypes::eNull:
        rec.value_= nullptr;
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
void Write(T &stream, const Field &rec)
{
    // Write the 8-bit type field
    
     if ( rec.value_ == nullptr)
    {
        //NULL is a parent type. every types derive for it
        DataTypes v = DataTypes::eNull;
        stream.write(reinterpret_cast<const char *>(&v), sizeof(v));
        return ;
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
            if (!::Read(file_stream_, record->fields_[i]))
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
        ::Write(file_stream_, it);
    }
    return true;
}

bool RecordFile::Delete(RecordId id)
{
    return false;
}