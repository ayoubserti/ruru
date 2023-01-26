// Copyright (c) 2023 Ayoub Serti
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef _H_STORAGE_ENGINE_HH_
#define _H_STORAGE_ENGINE_HH_

#include "btreeindex.h"
#include "ruru.h"
#include "resultset.h"

namespace ruru
{
  class Record;

  /*
    !!WIP
    StorageEngine : Represent a Table Storage
    It have one single index
  */

  class StorageEngine
  {
  public:
    // Constructor
    StorageEngine(const std::string &file_name);

    // Insert a record into the table
    void Insert(const Record &record);

    // Select all records from the table
    std::vector<Record> SelectAll();

    // Look up a record by key
    std::vector<Record> Lookup(const std::string &key);

    //Look up records by filter
    std::vector<RecordId> Lookup(const Filters_t& filters);

    // LoadRecord
    Record*          LoadRecord( RecordId id);

    // Save a record and set a record id
    bool Save( Record &record, bool isNew);

    // Flush 
    bool Flush();

  private:
    
    std::string file_name_;
    RecordId    current_rec_id_;
    BTreeIndex<std::string, int> index_;

    // row_id_index_ is a hidden index 
    // it allows quick retrieval and detection of deletion
    // if the record is deleted --> RecordLength_t = 0
    BTreeIndex<RecordId , std::pair<RecordLength_t,RecordPosition_t> > row_id_index_; 

    // Load the index from the index file
    void LoadIndex();

    // Save the index to the index file
    void SaveIndex();

    //Load hidden index
    void LoadHiddenIndex();


    //Load record by position
   RecordLength_t  _LoadRecord(RecordPosition_t position ,  Record& rec);
  };

}

#endif