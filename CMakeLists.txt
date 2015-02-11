cmake_minimum_required(VERSION 2.8.4)
project(HttpServer)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pedantic -Wall -Wextra -g -D_GLIBCXX_DEBUG -fsanitize=address")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} build/library)

SET(SOURCE_FILES main.cpp
                 HttpResponse.cpp
                 HttpRequest.cpp)

LINK_DIRECTORIES("build/library")

######SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)
######SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build)

#set(dir "${CMAKE_CURRENT_SOURCE_DIR}/build")

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${dir})
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${dir})
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${dir})

add_executable(HttpServer ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(HttpServer myserver)
