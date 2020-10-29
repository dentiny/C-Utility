#ifndef THREADSAFE_LOOKUP_TABLE_HPP__
#define THREADSAFE_LOOKUP_TABLE_HPP__

#include <map>
#include <list>
#include <mutex>
#include <vector>
#include <memory>
#include <algorithm>
#include <functional>
#include <shared_mutex>

template<typename Key, typename Value, typename Hash = std::hash<Key>>
class ThreadsafeLookupTable
{
public:
    using key_type = Key;
    using value_type = Value;
    using hash_type = Hash;

private:
    // every bucket consists of a list of kv pair
    class Bucket
    {
    public:
        using bucket_value = std::pair<Key, Value>;
        using bucket_data = std::list<bucket_value>;
        using bucket_iterator = typename bucket_data::iterator;

    private:
        bucket_iterator find_entry_for(const Key & key) const
        {
            return std::find_if(data.begin(), data.end(),
                [&](const bucket_value & item)
                { return item.first == key; });
        }

    public:
        Value value_for(const Key & key, const Value & default_value) const
        {
            std::shared_lock<std::shared_mutex> lck(mtx); // rw_lock
            bucket_iterator res = find_entry_for(key);
            return res == data.end() ? default_value : res->second;
        }

        void add_or_update_mapping(const Key & key, const Value & value)
        {
            std::unique_lock<std::shared_mutex> lck(mtx);
            bucket_iterator res = find_entry_for(key);
            if(res == data.end())
            {
                data.push_back(std::make_pair(key, value));
            }
            else
            {
                res->second = value;
            }
        }

        void remove_mapping(const Key & key)
        {
            std::unique_lock<std::shared_mutex> lck(mtx);
            bucket_iterator res = find_entry_for(key);
            if(res != data.end())
            {
                data.erase(res);
            }
        }

    public:
        bucket_data data; // list of kv pair
        mutable std::shared_mutex mtx;
    };  

private:
    Bucket & get_bucket(const Key & key) const
    {
        const unsigned bucket_idx = hasher(key) & buckets.size();
        return *buckets[bucket_idx];
    }

public:
    ThreadsafeLookupTable(unsigned bucket_size = 19, const Hash & _hasher = Hash{}) :
        hasher { _hasher },
        buckets(bucket_size)
    {
        for(unsigned idx = 0; idx < bucket_size; ++idx)
        {
            buckets[idx].reset(new Bucket);
        }
    }

    ThreadsafeLookupTable(const ThreadsafeLookupTable &) = delete;

    ThreadsafeLookupTable & operator=(const ThreadsafeLookupTable &) = delete;

    Value value_for(const Key & key, const Value & default_value = Value{}) const
    {
        return get_bucket(key).value_for(key, default_value);
    }

    void add_or_update_mapping(const Key & key, const Value & value)
    {
        get_bucket(key).add_or_update_mapping(key, value);
    }

    void remove_mapping(const Key & key)
    {
        get_bucket(key).remove_mapping(key);
    }

    std::map<Key, Value> get_map() const
    {
        // lock every bucket, equals to locking the whole map
        std::vector<std::unique_lock<std::shared_mutex>> lcks;
        for(unsigned idx = 0; idx < buckets.size(); ++idx)
        {
            lcks.emplace_back(buckets[idx].mtx);
        }

        std::map<Key, Value> res;
        for(unsigned idx = 0; idx < buckets.size(); ++idx)
        {
            for(auto it = buckets[idx].begin(); it != buckets[idx].end(); ++it)
            {
                res.insert(*it);
            }
        }
        return res;
    }

private:
    Hash hasher;
    std::vector<std::unique_ptr<Bucket>> buckets;
};

#endif