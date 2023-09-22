// Copyright (c) 2023 Ayoub Serti
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "pch.h"
#include "ruru.h"
#include "internal/basic_storage_with_cache.h"
#include "record.h"
#include "internal/RecordStream.h"
#include "internal/basic_store_cache.h"
#include "internal/tools.h"
using namespace ruru;
using namespace ruru::internal;


IStorageEngine* BasicCachedStorageEngineFactory::createStorageEngine( const std::string& file_name)
{
    return new BasicCachedStorageEngine(file_name);
}

std::string  BasicCachedStorageEngineFactory::getName( )
{
    return _basic_cached_factory;
}

//========================================================================================================
//                                      BasicCachedStorageEngine
//========================================================================================================

// Constructor
BasicCachedStorageEngine::BasicCachedStorageEngine(const std::string &file_name)
    : file_name_(file_name),
      current_rec_id_(-1),
      cache_store_(new CacheStore(file_name_))
{
    // Load the index from the index file
    LoadIndex();
    // Load row_id index
    LoadHiddenIndex();
    cache_store_->Load();

}

// Insert a record into the table
void BasicCachedStorageEngine::Insert(const Record &record)
{
    // Open the file in append mode
    std::fstream file(file_name_, std::ios::app);

    // Update the index
    index_.Insert(record.GetKey(), file.tellp());

    // update hidden index
    row_id_index_.Insert(record.row_id_, std::make_pair<RecordLength_t, RecordPosition_t>(record.GetRowSize(), file.tellp()));


    RecordFile record_file(file);
    record_file.Write(record);

    // Close the file
    file.close();
}

// Select all records from the table
std::vector<Record> BasicCachedStorageEngine::SelectAll()
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
std::vector<Record> BasicCachedStorageEngine::Lookup(const std::string &key)
{
  
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

std::vector<RecordId> BasicCachedStorageEngine::Lookup(const Filters_t &filters)
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

Record *BasicCachedStorageEngine::LoadRecord(RecordId id)
{
    if (!row_id_index_.Exists(id))
            return nullptr;
    auto entry = row_id_index_.Lookup(id);
    Record *rec = new Record();
    _LoadRecord(entry.second, *rec);
    return rec;
}

// Load the index from the index file
void BasicCachedStorageEngine::LoadIndex()
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
void BasicCachedStorageEngine::LoadHiddenIndex()
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
void BasicCachedStorageEngine::SaveIndex()
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
bool BasicCachedStorageEngine::Save(Record &record, bool isNew)
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

bool BasicCachedStorageEngine::Flush()
{
    SaveIndex();
    cache_store_->Flush();
    return true;
}

bool BasicCachedStorageEngine::DropStorage()
{
    return std::filesystem::remove(file_name_);
}

RecordLength_t BasicCachedStorageEngine::_LoadRecord(RecordPosition_t position, Record &rec)
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