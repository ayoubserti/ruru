// Copyright (c) 2023 Ayoub Serti
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef _H_BASIC_STORE_CACHE_HH_
#define _H_BASIC_STORE_CACHE_HH_

namespace ruru
{
    class Record;
}

namespace ruru::internal
{
    // constexps
    constexpr uint64_t SEGMENT_SIZE = 1024;     // size of each segment
    constexpr uint64_t SEGMENTS_PER_CACHE = 24; // number of segment per cache

    // forward declaration
    class CacheStore;

    //<interface>
    class ISegment
    {

    public:
        virtual bool Flush(const std::string &file_path) = 0;
        virtual bool GetRecord(RecordId id, Record &rec) = 0;
        virtual bool SetRecord(RecordId id, const Record &rec) = 0;
        virtual ~ISegment(){};
    };
    /*
        \class CacheSegment
        \brief represents the cache of a file portion
                it is related directly to the Cache

    */
    // template <uint64_t seg_size>
    class CacheSegment : public ISegment
    {
        // parent cache
        CacheStore *parent;
        // starting position
        std::size_t start;
        // end position
        std::size_t end;
        // managed rawids
        std::array<RecordId, SEGMENT_SIZE /*seg_size*/> rawIDSeg;
        // record Segment
        std::array<ruru::Record *, SEGMENT_SIZE /*seg_size*/> RecSeg;
        // data pos map
        std::unordered_map<RecordId, uint64_t> dataMap; // for each RecordId define its position in RegSeg
        // array cursor
        uint64_t cur_pos;
        friend class CacheStore;

        // private ctor
        CacheSegment(CacheStore *, std::size_t begin_pos, std::size_t end_pos);

        CacheSegment() = delete;

    public:
        bool Flush(const std::string &file_path) override;
        bool GetRecord(RecordId id, Record &rec) override;
        bool SetRecord(RecordId id, const Record &rec) override;
        virtual ~CacheSegment();
    };

    class CacheStore
    {
        std::string file_path;
        std::vector<ISegment *> segments;

    public:
        // ctor
        CacheStore(const std::string &file_path);

        // Load file into this cache. Cache have a limits, we only load the Max Segment ( SEGMENTS_PER_CACHE)
        bool Load();

        // Lookup
        std::vector<RecordId> Lookup(const Filters_t &filters);

        // Insert a record in the cache
        bool Insert(const Record &rec);

        // flush data into disk
        bool Flush();

        // dtor
        virtual ~CacheStore();
    };

}

#endif //_H_BASIC_STORE_CACHE_HH_