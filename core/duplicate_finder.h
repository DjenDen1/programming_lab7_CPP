#pragma once
#include "includer.h"
#include "core_settings.h"
#include "hash_func.h"
#include "block_reader.h"

namespace file_sys = boost::filesystem;

namespace duplicate_finder_detail
{
    struct work_item
    {
        std::vector<file_sys::path> files;
        uintmax_t block_index;
        uintmax_t file_size;
    };
};

inline std::vector<std::vector<file_sys::path>> find_duplicates(const std::vector<file_sys::path>& files, const block_settings& settings, const i_block_hasher& hasher)
{
    using duplicate_finder_detail::work_item;

    std::map<uintmax_t, std::vector<file_sys::path>> by_size;
    for(const auto& f : files)
    {
        boost::system::error_code ec;
        uintmax_t sz = file_sys::file_size(f,ec);

        if(ec) continue;

        by_size[sz].push_back(f);
    }

        std::deque<work_item> queue;
    for (auto& [size, group] : by_size)
    {
        if (group.size() > 1)
            queue.push_back(work_item{std::move(group), 0, size});
    }

    std::vector<std::vector<file_sys::path>> result;
    std::size_t block_size = settings.get_block_size();

     while (!queue.empty())
    {
        work_item item = std::move(queue.front());
        queue.pop_front();


        std::map<std::string, std::vector<file_sys::path>> buckets;
        for (const auto& f : item.files)
        {
            std::vector<char> block = read_block(f, item.block_index, settings);
            buckets[hasher.hash(block)].push_back(f);
        }

        uintmax_t next_block_index = item.block_index + 1;
        bool fully_consumed =
            (next_block_index * static_cast<uintmax_t>(block_size)) >= item.file_size;

        for (auto& [digest, bucket] : buckets)
        {
            if (bucket.size() < 2)
                continue;

            if (fully_consumed)
                result.push_back(std::move(bucket));
            else
                queue.push_back(work_item{std::move(bucket), next_block_index, item.file_size});
        }
    }

    return result;
}