cmake_minimum_required(VERSION 3.10.0)
project(lab7 VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)


find_package(Boost REQUIRED COMPONENTS program_options filesystem system)

add_executable(bayan
    main.cpp
    core/includer.h
    core/hash_func.h
    core/core_settings.h
    core/mask_filter.h
    core/scan_directory.h
    core/block_reader.h
    core/duplicate_finder.h
    program_struct/application.h
)
target_include_directories(bayan PRIVATE 
   ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/program_struct
    ${Boost_INCLUDE_DIRS}
)


target_link_libraries(bayan PRIVATE 
    Boost::program_options 
    Boost::filesystem 
    Boost::system
)