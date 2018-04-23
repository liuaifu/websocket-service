PROJECT(websocket-service)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
AUX_SOURCE_DIRECTORY(. DIR_SRCS)

cmake_policy(SET CMP0015 NEW)
include_directories(../third-party/libdbpool/include ../third-party/boost_1_67_0/include)
link_directories(../third-party/boost_1_67_0/lib-gcc6.3 ../third-party/libdbpool/lib)
add_compile_options(-std=c++11)
ADD_EXECUTABLE(${PROJECT_NAME} ${DIR_SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} -static -static-libgcc -static-libstdc++ dbpool mysqlclient boost_locale boost_timer boost_log boost_thread boost_program_options boost_filesystem boost_chrono boost_system pthread dl z m rt)
