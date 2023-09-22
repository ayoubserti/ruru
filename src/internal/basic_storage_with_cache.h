// Copyright (c) 2023 Ayoub Serti
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef _H_BASIC_STORAGE_WITH_CACHE_HH_
#define _H_BASIC_STORAGE_WITH_CACHE_HH_


#include "btreeindex.h"
#include "ruru.h"

namespace ruru
{

    namespace internal
    {
        //forward declaration
        class CacheStore;

        class BasicCachedStorageEngineFactory : public IStorageEngineFactory
        {

        public:
            virtual IStorageEngine *createStorageEngine(const std::string &name) override;
            virtual std::string getName() override;
        };

        class BasicCachedStorageEngine : public IStorageEngine
        {
        public:
            // Constructor
            BasicCachedStorageEngine(const std::string &file_name);

            // Insert a record into the table
            void Insert(const Record &record) override;

            // Select all records from the table
            std::vector<Record> SelectAll() override;

            // Look up a record by key
            std::vector<Record> Lookup(const std::string &key) override;

            // Look up records by filter
            std::vector<RecordId> Lookup(const Filters_t &filters) override;

            // LoadRecord
            Record *LoadRecord(RecordId id) override;

            // Save a record and set a record id
            bool Save(Record &record, bool isNew) override;

            // Flush
            bool Flush() override;

            // Drop Storage
            bool DropStorage() override;

            ~BasicCachedStorageEngine() = default;

        private:
            std::string file_name_;
            RecordId current_rec_id_;
            BTreeIndex<std::string, int> index_;

            // row_id_index_ is a hidden index
            // it allows quick retrieval and detection of deletion
            // if the record is deleted --> RecordLength_t = 0
            BTreeIndex<RecordId, std::pair<RecordLength_t, RecordPosition_t>> row_id_index_;

            // cache 
            std::unique_ptr<CacheStore>  cache_store_;

            // Load the index from the index file
            void LoadIndex();

            // Save the index to the index file
            void SaveIndex();

            // Load hidden index
            void LoadHiddenIndex();

            // Load record by position
            RecordLength_t _LoadRecord(RecordPosition_t position, Record &rec);
        };


    }
}

#endif