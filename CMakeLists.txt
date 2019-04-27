cmake_minimum_required(VERSION 3.12)
project(algorithm C)

set(CMAKE_C_STANDARD 11)

set(LIB_TARGET algorithm)
set(TEST_TARGET algorithm_test)

# 设置构建类型，及相应的编译选项
#set(CMAKE_BUILD_TYPE "Debug")
#set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_C_FLAGS_DEBUG "$ENV{CFLAGS} -O0 -Wall -g -ggdb -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "$ENV{CFLAGS} -O3 -Wall")
find_library(LIB_LIST m)

# 设置执行文件输出目录
set(EXECUTABLE_OUTPUT_PATH bin)

# 设置库输出路径
set(LIBRARY_OUTPUT_PATH lib)

# 头文件搜索目录
include_directories(include)
# 链接库搜索路径
#link_directories(lib)

# 添加源码
aux_source_directory(src SRCS)
aux_source_directory(test TEST_SRCS)

# 目标文件
add_library(${LIB_TARGET} ${SRCS})
target_link_libraries(${LIB_TARGET} ${LIB_LIST})
add_executable(${TEST_TARGET} ${TEST_SRCS})
target_link_libraries(${TEST_TARGET} ${LIB_TARGET})

# 添加依赖
#add_dependencies(${TEST_TARGET} ${LIB_TARGET})


# tlang
set(TLANG_TARGET tlang)
# bison and flex
find_package(BISON)
find_package(FLEX)
bison_target(YACC_TARGET tlang/tlang.y ${CMAKE_CURRENT_BINARY_DIR}/tlang_y.tab.c
        DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/tlang_y.tab.h
        )
flex_target(LEX_TARGET tlang/tlang.l ${CMAKE_CURRENT_BINARY_DIR}/tlang_lex.yy.c)

# default header file is y.tab.h
add_flex_bison_dependency(LEX_TARGET YACC_TARGET)

include_directories(tlang tlang/include ${CMAKE_CURRENT_BINARY_DIR})
aux_source_directory(tlang TLANG_SRCS)
add_executable(${TLANG_TARGET} ${TLANG_SRCS} ${BISON_YACC_TARGET_OUTPUTS} ${FLEX_LEX_TARGET_OUTPUTS})
target_link_libraries(${TLANG_TARGET} ${LIB_TARGET} readline)


# empty_lang
set(LANG_TARGET lang)
# bison and flex
find_package(BISON)
find_package(FLEX)
bison_target(YACC_TARGET2 empty_lang/lang.y ${CMAKE_CURRENT_BINARY_DIR}/lang_y.tab.c
        DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/lang_y.tab.h
        )
flex_target(LEX_TARGET2 empty_lang/lang.l ${CMAKE_CURRENT_BINARY_DIR}/lang_lex.yy.c)

# default header file is y.tab.h
add_flex_bison_dependency(LEX_TARGET2 YACC_TARGET2)

include_directories(empty_lang ${CMAKE_CURRENT_BINARY_DIR})
aux_source_directory(empty_lang LANG_SRCS)
add_executable(${LANG_TARGET} ${LANG_SRCS} ${BISON_YACC_TARGET2_OUTPUTS} ${FLEX_LEX_TARGET2_OUTPUTS})
target_link_libraries(${LANG_TARGET})
