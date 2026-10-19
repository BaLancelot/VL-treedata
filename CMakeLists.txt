cmake_minimum_required(VERSION 3.16)
project(VLTreeData LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(vl_storage
    src/storage/disk_manager.cc
    src/storage/buffer_pool.cc
    src/storage/lru_replacer.cc
    src/record/record_page.cc
    src/record/record_manager.cc
    src/index/bplus_internal_page.cc
    src/index/bplus_leaf_page.cc
)

target_include_directories(vl_storage PUBLIC src)

enable_testing()

add_executable(record_page_test tests/record_page_test.cc)
target_link_libraries(record_page_test PRIVATE vl_storage)

add_test(NAME record_page_test COMMAND record_page_test)
