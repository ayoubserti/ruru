// Copyright (c) 2023 Ayoub Serti
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef _H_BTREE_INDEX__
#define _H_BTREE_INDEX__

namespace ruru
{
    template <typename K, typename V>
    class BTreeIndex
    {
    public:
        // Insert a key-value pair into the index
        void Insert(const K &key, const V &value)
        {
            index_[key] = value;
        }

        bool Exists(const K& key)
        {
            return index_.find(key) != index_.end();
        }

        // Look up the value for a given key
        V Lookup(const K &key)
        {
            return index_[key];
        }

        // Delete the key-value pair for a given key
        void Delete(const K &key)
        {
            index_.erase(key);
        }

        // Get all the entries in the index
        std::vector<std::pair<K, V>> GetEntries()
        {
            std::vector<std::pair<K, V>> entries;
            for (const auto &kv : index_)
            {
                entries.push_back(kv);
            }
            return entries;
        }

        size_t GetSize()
        {
            return index_.size();
        }

        K GetMax()
        {
            return index_.rbegin()->first;
        }

        std::vector<K> GetKeys()
        {
            std::vector<K> result;
            size_t s = index_.size();
            result.resize(s);
            size_t i =0;
            for ( auto&& it: index_)
            {
                result[i] = it.first;
                i++;
            }
            return result;
        }

    private:
        std::map<K, V> index_;
    };
}

#endif