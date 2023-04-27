cmake_minimum_required(VERSION 3.16)

project(robots-client)


set (CMAKE_CXX_FLAGS "-std=gnu++20 -Wall -Wextra -Wconversion -Werror -O2 -pthread")

SET(Boost_USE_STATIC_LIBS FALSE)
FIND_PACKAGE(Boost REQUIRED COMPONENTS system program_options)
IF (Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
    LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
ENDIF (Boost_FOUND)


SET(libaries ${Boost_SYSTEM_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY})

add_executable(robots-client robots-client.cpp Buffer.cpp Connection.cpp)


TARGET_LINK_LIBRARIES(robots-client ${libaries})