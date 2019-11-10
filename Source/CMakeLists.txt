cmake_minimum_required(VERSION 3.14)

file(GLOB_RECURSE SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.*)
add_executable(JOpt ${SOURCE})
target_enable_ipo(JOpt)

target_include_directories(JOpt PRIVATE .)
target_link_libraries(JOpt PRIVATE NRT.Core) 
