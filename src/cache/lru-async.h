#pragma once

#include <list>
#include <mutex>
#include <utility>

namespace cache
{

template <typename key_t, typename data_t>
class LruAsync
{
    using cache_data_t   = std::shared_ptr<data_t>;
    using key_data_t     = std::pair<key_t, cache_data_t>;
    using data_list_it_t = typename std::list<key_data_t>::iterator;

public:
    LruAsync(size_t capacity)
        : capacity_(capacity)
    {
    }

    void put(const key_t& key, const data_t& value) &
    {
        std::lock_guard<std::mutex> lk(m_);

        auto dataIt = hash_.find(key);
        if (dataIt != hash_.cend())
            data_.erase(dataIt->second);

        if (data_.size() >= capacity_)
        {
            hash_.erase(data_.back().first);
            data_.pop_back();
        }

        data_.push_front({ key, std::make_shared<data_t>(value) });
        hash_[key] = data_.begin();
    }

    cache_data_t get(const key_t& key) &
    {
        std::lock_guard<std::mutex> lk(m_);

        auto dataIt = hash_.find(key);
        if (dataIt == hash_.cend())
            throw std::runtime_error("cache miss");

        data_.splice(data_.begin(), data_, dataIt->second);
        return dataIt->second->second;
    }

    std::optional<cache_data_t> try_get(const key_t& key) &
    {
        std::lock_guard<std::mutex> lk(m_);

        auto dataIt = hash_.find(key);
        if (dataIt == hash_.cend())
            return std::nullopt;

        data_.splice(data_.begin(), data_, dataIt->second);
        return dataIt->second->second;
    }

private:
    size_t                                    capacity_;
    std::list<key_data_t>                     data_;
    std::unordered_map<key_t, data_list_it_t> hash_;
    mutable std::mutex                        m_;
};

} // namespace cache
