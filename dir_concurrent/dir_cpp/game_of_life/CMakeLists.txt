cmake_minimum_required(VERSION 3.10)
project(GameOfLife)

set(CMAKE_CXX_STANDARD 17)

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS})

add_executable(GameOfLife main.cpp)

target_link_libraries(GameOfLife ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES} pthread)