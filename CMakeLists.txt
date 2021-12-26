cmake_minimum_required(VERSION 3.20)
project(dark_side)

set(CMAKE_CXX_STANDARD 14)

find_package(Boost REQUIRED COMPONENTS
        system
        program_options
        filesystem
        log
        log_setup)
find_package(PkgConfig REQUIRED)
pkg_check_modules(ODBSQLite REQUIRED libodb libodb-sqlite)
pkg_check_modules(Botan REQUIRED botan-2)

include_directories(${Boost_INCLUDE_DIRS} ${ODBSQLite_INCLUDE_DIRS} ${Botan_INCLUDE_DIRS})

add_library(dark_side STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/sources/user-odb.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/sources/database_redactor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/sources/convert.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/sources/encrypt.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/sources/database.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/sources/file_sharing.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/sources/client.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/sources/server.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/sources/log.cpp)

add_executable(app
        ${CMAKE_CURRENT_SOURCE_DIR}/app/main.cpp
        )

target_include_directories(dark_side PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:include>"
        )

target_link_libraries(dark_side
        ${Boost_LIBRARIES}
        ${ODBSQLite_LIBRARIES}
        ${Botan_LIBRARIES})

target_link_libraries(app dark_side)
