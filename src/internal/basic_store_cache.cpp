#include "pch.h"
#include "ruru.h"
#include "record.h"
#include "internal/basic_store_cache.h"
#include "internal/basic_storage_engine.h"
#include "internal/RecordStream.h"
#include "utils/binary_stream.h"

namespace ruru::internal
{

    // template<uint64_t seg_size>
    CacheSegment::CacheSegment(CacheStore *parent, std::size_t begin_pos, std::size_t end_pos)
        : parent(parent), start(begin_pos), end(end_pos), cur_pos(-1)
    {
    }

    bool CacheSegment::Flush(const std::string &file_path)
    {
        bool result = true;
        std::streampos current_pos = start;
        std::fstream out(file_path, std::ios::binary | std::ios::app);
        if (!out.is_open())
        {
            return false;
        }

        out.seekp(current_pos);
        for (auto &&it : rawIDSeg)
        {
            if (dataMap.find(it) != dataMap.end())
            {
                uint64_t indice = dataMap[it];
                Record *rec = RecSeg[indice];
                if (rec != nullptr)
                {
                    assert(rec->row_id_ == it);
                    assert(rec->GetRowSize() + current_pos <= end);
                    RecordFile rec_file(out);
                    result = rec_file.Write(*rec);
                    if (result != true)
                        return false;
                }
            }
        }

        out.close();
        return result;
    }

    bool CacheSegment::GetRecord(RecordId id, Record &rec)
    {
        bool result = true;
        if (dataMap.find(id) == dataMap.end())
            return false;

        auto indice = dataMap[id];
        rec = *RecSeg[indice];

        return result;
    }

    bool CacheSegment::SetRecord(RecordId id, const Record &rec)
    {
        bool result = true;
        if (dataMap.find(id) == dataMap.end())
            cur_pos++;
        Record *new_rec = new Record(rec);
        RecSeg[cur_pos] = new_rec;
        dataMap[id] = cur_pos;
        rawIDSeg[cur_pos] = id;

        return result;
    }

    CacheSegment::~CacheSegment()
    {
        for (auto &&it : RecSeg)
            delete it;
    }

#pragma region CacheStore

    CacheStore::CacheStore(const std::string &file_path)
        : file_path(file_path)
    {
    }

    bool CacheStore::Load()
    {
        // Load from the file
        std::fstream in_file(file_path, std::ios::in | std::ios::binary);
        if (!in_file.is_open())
            return false;

        std::size_t cur_pos, start_pos;

        cur_pos = 0;
        start_pos = 0;
        CacheSegment *seg = new CacheSegment(this, start_pos, cur_pos);

        // char buffer[1024 * 1024]; // 1Mb
        std::array<char, 1024 * 1024> buffer;
        while (true)
        {

            std::fill(buffer.begin(), buffer.end(), 0);

            if ((int64_t)seg->cur_pos >= (int64_t)SEGMENT_SIZE)
            {
                seg->end = in_file.tellp();
                seg->end--;
                start_pos = in_file.tellp();
                segments.push_back(seg);
                CacheSegment *seg = new CacheSegment(this, start_pos, cur_pos);
            }
            in_file.read(buffer.begin(), buffer.size());

            BinaryStream<decltype(buffer)> stream(buffer);
            std::streamsize ss = in_file.gcount();
            if (ss <= 0)
                break;

            while (!stream.eof() && !stream.fail() && stream.tell() < ss) 
            {
                RecordStream<BinaryStream<decltype(buffer)>> rec_stream(stream);
                RecordId id = -1;
                Record rec;
                if (rec_stream.Read(id, &rec))
                {
                    seg->SetRecord(rec.row_id_, rec);
                    cur_pos += rec.GetRowSize();
                }
                else
                {
                    // a stream error occurs
                    // meaning not all the rec is loaded from disk
                    // continue loading next part of the rec
                    std::streamsize remind = stream.tell();
                    throw std::runtime_error("Not impement");
                }
            }
            int a = 0;
        }
        segments.push_back(seg);

        in_file.close();
        return true;
    }

    std::vector<RecordId> CacheStore::Lookup(const Filters_t &filters)
    {
        std::vector<RecordId> result;

        return result;
    }

    // Insert a record in the cache
    bool CacheStore::Insert(const Record &rec)
    {
        return false;
    }

    // flush data into disk
    bool CacheStore::Flush()
    {
        return false;
    }

    // dtor
    CacheStore::~CacheStore()
    {
        for (auto &&it : segments)
            delete it;
    }

#pragma endregion

}
