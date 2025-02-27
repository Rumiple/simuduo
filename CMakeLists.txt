cmake_minimum_required(VERSION 3.10) # 要求当前环境的CMake编译版本要高于2.5才能编译
project(simuduo) # 执行

# set(CMAKE_C_COMPILER "/usr/local/bin/gcc")
# set(CMAKE_CXX_COMPILER "/usr/local/bin/g++")

# set(CMAKE_C_COMPILER /usr/bin/gcc)
# set(CMAKE_CXX_COMPILER /usr/bin/g++)

# muduo最终编译成so动态库，设置动态库的路径为当前根目录下的lib文件夹
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib) 
# 设置调试信息以及启动c++11语言标准
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -fPIC") 

# 定义参与编译的源代码文件，将所有打包成SRC_LIST变量里。"."表示根目录下的所有源文件。
aux_source_directory(. SRC_LIST)

# 编译生成动态库muduo。SHARED表示动态。
add_library(simuduo SHARED ${SRC_LIST})


