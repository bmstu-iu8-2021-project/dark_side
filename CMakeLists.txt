cmake_minimum_required(VERSION 3.20)
project(dark_side)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED COMPONENTS system thread)
find_package(SQLite3)

include_directories(${Boost_INCLUDE_DIR} ${SQLite3_INCLUDE_DIR})

add_library(${PROJECT_NAME} STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/sources/source.cpp
        )

add_executable(app
        ${CMAKE_CURRENT_SOURCE_DIR}/app/main.cpp
        )

target_include_directories(${PROJECT_NAME} PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:include>"
        )

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} ${SQLite3_LIBRARY})
target_link_libraries(app ${PROJECT_NAME})
