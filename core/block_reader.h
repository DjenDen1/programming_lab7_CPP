#pragma once

#include "includer.h"
#include "core_settings.h"

namespace file_sys = boost::filesystem;

inline std::vector<char> read_block(const file_sys::path& path, uintmax_t block_index, const block_settings& settings)
{
    std::size_t size = settings.get_block_size();
    std::vector<char> buffer(size, '\0');

    std::ifstream ifs(path.string(), std::ios::binary);
    if(!ifs) throw std::runtime_error("cannot open file for reading: " + path.string());

    ifs.seekg(static_cast<std::streamoff>(block_index * size));

    if(ifs) ifs.read(buffer.data(), static_cast<std::streamsize>(size));

    return buffer;

}