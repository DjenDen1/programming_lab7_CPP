#include "includer.h"
#include "mask_filter.h"

namespace file_sys = boost::filesystem;

namespace file_utilities
{
    inline file_sys::path to_cannonical(const file_sys::path& path)
    {
        boost::system::error_code ec;
        file_sys::path c = file_sys::canonical(path, ec);

        return ec ? file_sys::absolute(path) : c;
    }

    inline bool is_subpath(const file_sys::path& candidate, const file_sys::path& root)
    {
        auto candidate_index = candidate.begin();
        auto root_index = root.begin();

        for (; root_index != root.end(); ++root_index, ++candidate_index)
        {
            if (candidate_index == candidate.end() || *candidate_index != *root_index) return false;
        }

        return true;
    }
}

class exception_directories
{
private:
    std::vector<file_sys::path> directories;

public:
    ~exception_directories() = default;
    exception_directories() = default;

    void add_directory(const file_sys::path& dir)
    {
        directories.push_back(file_utilities::to_cannonical(dir));
    }

    const std::vector<file_sys::path>& get_directories() const
    {
        return directories;
    }

    bool is_excluded(const file_sys::path& candidate) const
    {
        file_sys::path canon = file_utilities::to_cannonical(candidate);

        for (const auto& root : directories)
        {
            if (file_utilities::is_subpath(canon, root))
                return true;
        }
        return false;
    }
};

class scanned_directories
{
private:
    std::vector<file_sys::path> directories;
    long int level_scan;

public:
    ~scanned_directories() = default;
    explicit scanned_directories(long int level = -1) : level_scan(level) {}

    void add_directory(const file_sys::path& dir)
    {
        directories.push_back(file_utilities::to_cannonical(dir));
    }

    const std::vector<file_sys::path>& get_directories() const
    {
        return directories;
    }

    long int get_level() const
    {
        return level_scan;
    }

    void set_level(long int number)
    {
        level_scan = number;
    }
};

namespace file_utilities
{
     inline void scan_recursive(const file_sys::path& dir, long int depth, long int max_level, const exception_directories& ex_dirs, std::vector<file_sys::path>& result)
    {
        boost::system::error_code ec;
        file_sys::directory_iterator it(dir, ec), end;

        if (ec)
        {
            std::cerr << "ERROR: cannot open directory " << dir
                      << ": " << ec.message() << "\n";
            return;
        }

        for (; it != end; it.increment(ec))
        {
            if (ec) break;

            const file_sys::path& p = it->path();

            if (ex_dirs.is_excluded(p))
                continue;

            boost::system::error_code stat_ec;
            if (file_sys::is_directory(p, stat_ec) && !stat_ec)
            {
                if (max_level < 0 || depth < max_level)
                    scan_recursive(p, depth + 1, max_level, ex_dirs, result);
            }
            else if (file_sys::is_regular_file(p, stat_ec) && !stat_ec)
            {
                result.push_back(p);
            }
        }
    }
}

std::vector<file_sys::path> scan_directory(const scanned_directories& sc_dirs,
                                            const exception_directories& ex_dirs)
{
    std::vector<file_sys::path> result;

    for (const auto& dir : sc_dirs.get_directories())
    {
        boost::system::error_code ec;

        if (!file_sys::exists(dir, ec) || !file_sys::is_directory(dir, ec))
        {
            std::cerr << "WARNING: skipping directory -> " << dir << "\n";
            continue;
        }

        if (ex_dirs.is_excluded(dir))
            continue;

        file_utilities::scan_recursive(dir, 0, sc_dirs.get_level(), ex_dirs, result);
    }

    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());

    return result; 
}