cmake_minimum_required(VERSION 3.22)
project(server-drone8)

set(CMAKE_CXX_STANDARD 17)


file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/area_snapshots")

add_executable(server main.cpp
        Server.cpp
        Server.hpp
        Session.hpp
        Session.cpp
        ../con2db/pgsql.cpp
        ../con2db/pgsql.h

)
# Includi la directory degli header di PostgreSQL
target_include_directories(server PRIVATE "/usr/include/postgresql")
# Includi la directory degli header di nlohmann/json
target_include_directories(server PRIVATE "/usr/include/nlohmann")

target_include_directories(server PRIVATE "/usr/local/include/sw")

target_include_directories(server PRIVATE "/usr/include/curl")

target_include_directories(server PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(server PRIVATE pq curl ${Boost_LIBRARIES})