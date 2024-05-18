#cmake_minimum_required(VERSION 3.23)
#project(drone8)
#
#set(CMAKE_CXX_STANDARD 17)
#
## Definisci le directory di inclusione comuni
#set(COMMON_INCLUDE_DIRS
#        "/usr/include/hiredis"
#        "/usr/include/postgresql"
#        "/usr/include/nlohmann"
#        "/usr/local/include/sw"
#)
#
## Definisci le librerie comuni
#set(COMMON_LIBRARIES hiredis pq redis++)
#
## Funzione per creare gli eseguibili
#function(create_executable target_name)
#    add_executable(${target_name} ${ARGN})
#    target_include_directories(${target_name} PRIVATE ${COMMON_INCLUDE_DIRS})
#    target_link_libraries(${target_name} PRIVATE ${COMMON_LIBRARIES})
#endfunction() #
## Esegui la creazione degli eseguibili
#create_executable(cc
#        src/control_center/main.cpp
#        src/control_center/ControlCenter.cpp src/drone/Drone.cpp
#        src/scanning_strategy/SimpleStrategy.cpp
#        src/area/Area.cpp
#        src/redis/StreamRedis.cpp
#        src/redis/redis.cpp
#)
#
#create_executable(drone
#        src/drone/main.cpp
#        src/drone/Drone.cpp
#        src/scanning_strategy/SimpleStrategy.cpp
#        src/area/Area.cpp
#        src/redis/StreamRedis.cpp
#        src/redis/redis.cpp
#)
#
#create_executable(test
#        main.cpp
#        src/control_center/ControlCenter.cpp
#        src/drone/Drone.cpp
#        src/scanning_strategy/SimpleStrategy.cpp
#        src/area/Area.cpp
#        src/redis/StreamRedis.cpp
#        src/redis/redis.cpp
#)
#
## Aggiungi le dipendenze tra gli eseguibili
#add_dependencies(cc drone)
#add_dependencies(test cc)



cmake_minimum_required(VERSION 3.22)
project(drone8)

set(CMAKE_CXX_STANDARD 17)



add_executable(cc src/control_center/main.cpp
        src/control_center/ControlCenter.cpp
        src/control_center/ControlCenter.hpp
        src/drone/Drone.cpp
        src/drone/Drone.h
        src/scanning_strategy/ScanningStrategy.h
        src/area/Area.cpp
        src/area/Area.hpp
        src/redis/StreamRedis.cpp
        src/redis/StreamRedis.h
        src/redis/redis.cpp
        src/redis/redis.h
        src/con2db/pgsql.cpp
        src/con2db/pgsql.h
        src/scanning_strategy/Path.h
        src/scanning_strategy/Path.cpp
        src/scanning_strategy/BasicStrategy.cpp
        src/scanning_strategy/BasicStrategy.h
        src/config.h
        src/config.cpp
)




# CONTROL CENTER
target_include_directories(cc PRIVATE "/usr/include/hiredis")
target_include_directories(cc PRIVATE "/usr/include/postgresql")
target_include_directories(cc PRIVATE "/usr/include/nlohmann")
target_include_directories(cc PRIVATE "/usr/include/curl")
target_include_directories(cc PRIVATE ${Boost_INCLUDE_DIRS})


target_link_libraries(cc hiredis pq curl ${Boost_LIBRARIES})

# DRONE
add_executable(drone src/drone/main.cpp
        src/drone/Drone.cpp
        src/drone/Drone.h
        src/scanning_strategy/ScanningStrategy.h
        src/scanning_strategy/BasicStrategy.cpp
        src/scanning_strategy/BasicStrategy.h
        src/area/Area.cpp
        src/area/Area.hpp
        src/redis/StreamRedis.cpp
        src/redis/StreamRedis.h
        src/drone/main.cpp
        src/redis/redis.cpp
        src/redis/redis.h
        src/con2db/pgsql.cpp
        src/con2db/pgsql.h
        src/scanning_strategy/Path.h
        src/scanning_strategy/Path.cpp
        src/config.h
        src/config.cpp
)

target_include_directories(drone PRIVATE "/usr/include/hiredis")
target_include_directories(drone PRIVATE "/usr/include/postgresql")
target_include_directories(drone PRIVATE "/usr/include/nlohmann")
target_include_directories(drone PRIVATE "/usr/include/curl")
target_include_directories(drone PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(drone hiredis pq curl ${Boost_LIBRARIES})

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/area_snapshots")

add_executable(server src/server/main.cpp
       src/server/Server.cpp
       src/server/Server.hpp
       src/server/Session.hpp
       src/server/Session.cpp
       src/con2db/pgsql.cpp
       src/con2db/pgsql.h
)
# Includi la directory degli header di PostgreSQL
target_include_directories(server PRIVATE "/usr/include/postgresql")
# Includi la directory degli header di nlohmann/json
target_include_directories(server PRIVATE "/usr/include/nlohmann")

target_include_directories(server PRIVATE "/usr/local/include/sw")

target_include_directories(server PRIVATE "/usr/include/curl")

target_include_directories(server PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(server PRIVATE pq curl ${Boost_LIBRARIES})




# Testing
add_executable(test main.cpp
        src/control_center/ControlCenter.cpp
        src/control_center/ControlCenter.hpp
        src/drone/Drone.cpp
        src/drone/Drone.h
        src/scanning_strategy/ScanningStrategy.h
        src/scanning_strategy/BasicStrategy.cpp
        src/scanning_strategy/BasicStrategy.h
        src/area/Area.cpp
        src/area/Area.hpp
        src/redis/StreamRedis.cpp
        src/redis/StreamRedis.h
        src/redis/redis.cpp
        src/redis/redis.h
        src/con2db/pgsql.cpp
        src/con2db/pgsql.h
        src/scanning_strategy/Path.h
        src/scanning_strategy/Path.cpp
        src/config.h
        src/config.cpp
        src/server/Server.cpp
        src/server/Server.hpp
        src/server/Session.hpp
        src/server/Session.cpp
)
# Includi la directory degli header di Hiredis
target_include_directories(test PRIVATE "/usr/include/hiredis")
# Includi la directory degli header di PostgreSQL
target_include_directories(test PRIVATE "/usr/include/postgresql")
# Includi la directory degli header di nlohmann/json
target_include_directories(test PRIVATE "/usr/include/nlohmann")

target_include_directories(test PRIVATE "/usr/local/include/sw")

target_include_directories(test PRIVATE "/usr/include/curl")

target_include_directories(test PRIVATE ${Boost_INCLUDE_DIRS})

# Linka le librerie Hiredis e PostgreSQL
target_link_libraries(test PRIVATE hiredis pq curl ${Boost_LIBRARIES})

add_dependencies(cc drone)

add_dependencies(test cc)

