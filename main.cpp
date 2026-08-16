#include "core/core_settings.h"
#include "core/hash_func.h"
#include "core/mask_filter.h"
#include "core/scan_directory.h"



int main(int argc, char* argv[]) {
    namespace po = boost::program_options;
    
    try {
        po::options_description desc("Options");
        desc.add_options()
            ("help,h", "Show help")
            ("include,I", po::value<std::vector<std::string>>()->required(), 
             "Directories to scan (can be specified multiple times)")
            ("exclude,E", po::value<std::vector<std::string>>(), 
             "Directories to exclude")
            ("level,L", po::value<int>()->default_value(-1), 
             "Scan level (0 - only specified directory)")
            ("min-size,M", po::value<uintmax_t>()->default_value(1), 
             "Minimum file size")
            ("mask,m", po::value<std::vector<std::string>>(), 
             "File name masks")
            ("block-size,S", po::value<std::size_t>()->default_value(4096), 
             "Block size")
            ("hash,H", po::value<std::string>()->default_value("crc32"), 
             "Hash algorithm (crc32 or md5)");
        
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
        
        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 0;
        }
        
        // Создаем объекты для сканирования
        scanned_directories sc_dirs(vm["level"].as<int>());
        for (const auto& dir : vm["include"].as<std::vector<std::string>>()) {
            sc_dirs.add_directory(dir);
        }
        
        exception_directories ex_dirs;
        if (vm.count("exclude")) {
            for (const auto& dir : vm["exclude"].as<std::vector<std::string>>()) {
                ex_dirs.add_directory(dir);
            }
        }
        
        // Сканируем директории
        auto files = scan_directory(sc_dirs, ex_dirs);
        
        // Выводим найденные файлы (для тестирования)
        std::cout << "Found " << files.size() << " files:\n";
        for (const auto& file : files) {
            std::cout << file << "\n";
        }
        
        // Здесь будет поиск дубликатов...
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}