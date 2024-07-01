cmake_minimum_required(VERSION 3.25)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

# 获取项目的根路径的最后一个文件夹的名称
get_filename_component(ProjectName ${CMAKE_SOURCE_DIR} NAME)
# 替换名称中的空格为下划线
string(REPLACE " " "_" ProjectName ${ProjectName})

# 打印项目名称用于调试
#message("Project Name: ${ProjectName}")

# 设置项目名称
project(${ProjectName} CXX)

set(PROJECT_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/bin)

include_directories(${PROJECT_INCLUDE_DIRS})

# 获取所有的源文件
file(GLOB_RECURSE source_files ${CMAKE_SOURCE_DIR}/src/*.cpp)

# 创建可执行文件
add_executable(${ProjectName} ${source_files})
