# 设置 CMake 的最小版本
cmake_minimum_required(VERSION 3.22)
# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# 指定 C 和 C++ 编译器
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)


# Project Name Setting
# 获取项目的根路径的最后一个文件夹的名称
get_filename_component(project_name ${CMAKE_SOURCE_DIR} NAME)
# 空格和-替换为下划线
string(REPLACE " " "_" project_name ${project_name})
string(REPLACE "-" "_" project_name ${project_name})
project(${project_name} CXX)


# Set Program include directory
# my common header
# 自己写的公共头文件的路径，没有可以删除
set(COMMON_INCLUDE_DIRS)
set(PROJECT_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include)
include_directories(${COMMON_INCLUDE_DIRS} ${PROJECT_INCLUDE_DIRS})


# LIBRARY_OUTPUT_PATH 以及 EXECUTABLE_OUTPUT_PATH 都是旧变量，现在又更新的方式指定
# LIBRARY_OUTPUT_PATH包括了静态库和动态库
# CMAKE_ARCHIVE_OUTPUT_DIRECTORY 是静态库；CMAKE_LIBRARY_OUTPUT_DIRECTORY是动态库
# CMAKE_RUNTIME_OUTPUT_DIRECTORY 是新版的 EXECUTABLE_OUTPUT_PATH
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/lib)


# 为全部的编译项目添加编译参数
# 设计详细的警告 将错误提到编译期
add_compile_options(
        -Wall
        -Wextra
        -Wconversion
        -Wsign-compare
        -Wzero-as-null-pointer-constant
        -Wold-style-cast
        -Wnon-virtual-dtor
        -Werror
)


# 获取所有的源文件
file(GLOB_RECURSE source_files ${CMAKE_SOURCE_DIR}/src/*.cpp)
# 排除main.cpp
# 这是正则表达式 同时\也需要转义成为 \\
list(FILTER source_files EXCLUDE REGEX "\\/main\\.cpp$")


# add required libiary
# 必须一个库一行 不能把所有的库写到一起
find_package(Catch2 REQUIRED)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)

# 创建动态库 然后链接
# 将src中 除了main.cpp的所有文件编译成一个动态库
# 后面仅需要链接两次，而不需要每次编译的时候都指明一堆编译的文件
add_library(${project_name}_lib SHARED ${source_files})
target_link_libraries(${project_name}_lib PRIVATE fmt::fmt)
target_link_libraries(${project_name}_lib PRIVATE spdlog::spdlog)


# 创建可执行文件 只需要编译main
add_executable(${project_name} ${CMAKE_SOURCE_DIR}/src/main.cpp)
target_link_libraries(${project_name} PRIVATE ${project_name}_lib)
target_link_libraries(${project_name} PRIVATE fmt::fmt)
target_link_libraries(${project_name} PRIVATE spdlog::spdlog)


# 创建测试可执行文件
add_executable(${project_name}_test ${CMAKE_SOURCE_DIR}/test/test.cpp)
target_link_libraries(${project_name}_test PRIVATE ${project_name}_lib)
target_link_libraries(${project_name}_test PRIVATE Catch2::Catch2WithMain)
target_link_libraries(${project_name}_test PRIVATE fmt::fmt)
target_link_libraries(${project_name}_test PRIVATE spdlog::spdlog)


# 选择性的为特定目标添加编译选项
# target_compile_options(<TARGET> PRIVATE ...)
# ...


# 手动编译test里面的一些其他组件
# stress test
# add_executable(${project_name}_stress_test ${CMAKE_SOURCE_DIR}/test/stress_test.cpp)


# 进行Cmake测试
enable_testing()
add_test(NAME run_${project_name}_test COMMAND ${project_name}_test)

