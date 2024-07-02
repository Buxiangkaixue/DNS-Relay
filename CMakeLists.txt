cmake_minimum_required(VERSION 3.25)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

# 获取项目的根路径的最后一个文件夹的名称
get_filename_component(ProjectName ${CMAKE_SOURCE_DIR} NAME)
# 替换名称中的空格为下划线
string(REPLACE " " "_" ProjectName ${ProjectName})

# 设置项目名称
project(${ProjectName} CXX)

set(PROJECT_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/lib)

include_directories(${PROJECT_INCLUDE_DIRS})

# 添加依赖
find_package(Catch2 REQUIRED)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)

# 获取所有的源文件
file(GLOB_RECURSE source_files ${CMAKE_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE test_files ${CMAKE_SOURCE_DIR}/test/*.cpp)

# 排除main.cpp
list(FILTER source_files EXCLUDE REGEX "main\\.cpp$")

# 创建动态库
add_library(${ProjectName}_lib SHARED ${source_files})

# 链接动态库依赖
target_link_libraries(${ProjectName}_lib PRIVATE fmt::fmt)
target_link_libraries(${ProjectName}_lib PRIVATE spdlog::spdlog)

# 创建可执行文件
add_executable(${ProjectName} ${CMAKE_SOURCE_DIR}/src/main.cpp
        src/UpStreamDNS.cpp
        include/UpStreamDNS.h
        src/Config.cpp
        include/Config.h
        src/NetworkService.cpp
        include/NetworkService.h
        src/DNSResolver.cpp
        include/DNSResolver.h
        src/DNSCache.cpp
        include/DNSCache.h
        src/DNSDatabase.cpp
        include/DNSDatabase.h src/DNS_utils.cpp include/DNS_utils.h)
target_link_libraries(${ProjectName} PRIVATE ${ProjectName}_lib)
target_link_libraries(${ProjectName} PRIVATE fmt::fmt)
target_link_libraries(${ProjectName} PRIVATE spdlog::spdlog)

# 创建测试可执行文件
add_executable(${ProjectName}_test ${test_files})
target_link_libraries(${ProjectName}_test PRIVATE ${ProjectName}_lib)
target_link_libraries(${ProjectName}_test PRIVATE Catch2::Catch2WithMain)
target_link_libraries(${ProjectName}_test PRIVATE fmt::fmt)
target_link_libraries(${ProjectName}_test PRIVATE spdlog::spdlog)

# 设置输出路径
set_target_properties(${ProjectName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})
set_target_properties(${ProjectName}_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})
set_target_properties(${ProjectName}_lib PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_PATH})
