#pragma once
#include "includer.h"
#include "core_settings.h"
#include "hash_func.h"
#include "mask_filter.h"
#include "scan_directory.h"
#include "block_reader.h"
#include "duplicate_finder.h"

namespace po = boost::program_options;
namespace file_sys = boost::filesystem;

class bayan_application
{
private:
    po::options_description desc;
    po::variables_map vm;

    static po::options_description build_options_description()
    {
        po::options_description d("Options");
        d.add_options()
            ("help,h", "Show help")
            ("include,I", po::value<std::vector<std::string>>()->required(), "Directories to scan")
            ("exclude,E", po::value<std::vector<std::string>>(), "Directories to exclude")
            ("level,L", po::value<int>()->default_value(-1), "Scan level (0 - only specified directory)")
            ("min-size,M", po::value<uintmax_t>()->default_value(1), "Minimum file size")
            ("mask,m", po::value<std::vector<std::string>>(), "File name masks")
            ("block-size,S", po::value<std::size_t>()->default_value(4096), "Block size")
            ("hash,H", po::value<std::string>()->default_value("crc32"), "Hash algorithm (crc32 or md5)");
        return d;
    }

    bool parse_arguments(int argc, char* argv[])
    {
        po::store(po::parse_command_line(argc, argv, desc), vm);

        if (vm.count("help"))
        {
            std::cout << desc << "\n";
            return false;
        }

        po::notify(vm);
        return true;
    }

    scanned_directories build_scanned_directories() const
    {
        scanned_directories sc_dirs(vm["level"].as<int>());
        for (const auto& dir : vm["include"].as<std::vector<std::string>>())
            sc_dirs.add_directory(dir);
        return sc_dirs;
    }

    exception_directories build_exception_directories() const
    {
        exception_directories ex_dirs;
        if (vm.count("exclude"))
            for (const auto& dir : vm["exclude"].as<std::vector<std::string>>())
                ex_dirs.add_directory(dir);
        return ex_dirs;
    }

    name_mask_filter build_mask_filter() const
    {
        name_mask_filter mask_filter;
        if (vm.count("mask"))
            for (const auto& m : vm["mask"].as<std::vector<std::string>>())
                mask_filter.add_mask(m);
        return mask_filter;
    }

    // scan_directory отдаёт все файлы без учёта размера/маски -
    // применяем оба фильтра здесь, после сканирования
    static std::vector<file_sys::path> apply_filters(
        const std::vector<file_sys::path>& files,
        const filter_byte& byte_filter,
        const name_mask_filter& mask_filter)
    {
        std::vector<file_sys::path> result;
        result.reserve(files.size());

        for (const auto& f : files)
        {
            if (!mask_filter.passes(f.filename().string()))
                continue;

            boost::system::error_code ec;
            uintmax_t sz = file_sys::file_size(f, ec);
            if (ec) continue;
            if (!byte_filter.passes(sz)) continue;

            result.push_back(f);
        }

        return result;
    }

    static void print_groups(const std::vector<std::vector<file_sys::path>>& groups)
    {
        bool first = true;
        for (const auto& group : groups)
        {
            if (!first) std::cout << "\n";
            first = false;
            for (const auto& p : group)
                std::cout << p.string() << "\n";
        }
    }

public:
    bayan_application() : desc(build_options_description()) {}

    int run(int argc, char* argv[])
    {
        try
        {
            if (!parse_arguments(argc, argv))
                return 0;

            scanned_directories sc_dirs = build_scanned_directories();
            exception_directories ex_dirs = build_exception_directories();
            filter_byte byte_filter(vm["min-size"].as<uintmax_t>());
            name_mask_filter mask_filter = build_mask_filter();

            // scan_directory - только 2 аргумента, как в исходной версии
            auto all_files = scan_directory(sc_dirs, ex_dirs);
            auto files = apply_filters(all_files, byte_filter, mask_filter);

            block_settings blk_settings(vm["block-size"].as<std::size_t>());
            auto hasher = make_hasher(vm["hash"].as<std::string>());

            auto groups = find_duplicates(files, blk_settings, *hasher);

            for (auto& g : groups) std::sort(g.begin(), g.end());
            std::sort(groups.begin(), groups.end(),
                      [](const auto& a, const auto& b) { return a.front() < b.front(); });

            print_groups(groups);
            return 0;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error: " << e.what() << "\n";
            return 1;
        }
    }
};