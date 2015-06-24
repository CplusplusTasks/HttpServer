cmake_minimum_required(VERSION 2.8.4)
project(Tic_Tac_Toe)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x -pedantic -Wall -Wextra -Wno-unused-result -Wformat=2 -Wfloat-equal -Wconversion -Wlogical-op -DDEBUG -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_FORTIFY_SOURCE=2 -fsanitize=address -g")

INCLUDE_DIRECTORIES(src headers)

SET(SOURCE_FILES src/main.cpp
                 src/HttpResponse.cpp
                 src/HttpRequest.cpp
                 src/TcpServer.cpp
                 src/TcpSocket.cpp
                 src/TcpClient.cpp
                 src/EpollLoop.cpp
                 src/HttpClient.cpp
                 src/HttpServer.cpp
                 src/GameServer.cpp)


set(dir "${CMAKE_CURRENT_SOURCE_DIR}/build")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${dir})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${dir})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(Tic_Tac_Toe ${SOURCE_FILES})
