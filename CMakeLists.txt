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
        src/control_center/ThreadPool.h
        src/control_center/ThreadPool.h
        src/control_center/ThreadPool.cpp
)




# CONTROL CENTER
target_include_directories(cc PRIVATE "/usr/include/hiredis")
target_include_directories(cc PRIVATE "/usr/include/postgresql")
target_include_directories(cc PRIVATE "/usr/include/nlohmann")
target_include_directories(cc PRIVATE "/usr/include/curl")


target_link_libraries(cc hiredis pq curl)

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
        src/control_center/ThreadPool.h
        src/control_center/ThreadPool.cpp
)

target_include_directories(drone PRIVATE "/usr/include/hiredis")
target_include_directories(drone PRIVATE "/usr/include/postgresql")
target_include_directories(drone PRIVATE "/usr/include/nlohmann")
target_include_directories(drone PRIVATE "/usr/include/curl")

target_link_libraries(drone hiredis pq curl)





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
        src/control_center/ThreadPool.h
        src/control_center/ThreadPool.cpp
)
# Includi la directory degli header di Hiredis
target_include_directories(test PRIVATE "/usr/include/hiredis")
# Includi la directory degli header di PostgreSQL
target_include_directories(test PRIVATE "/usr/include/postgresql")
# Includi la directory degli header di nlohmann/json
target_include_directories(test PRIVATE "/usr/include/nlohmann")

target_include_directories(test PRIVATE "/usr/local/include/sw")

target_include_directories(test PRIVATE "/usr/include/curl")

# Linka le librerie Hiredis e PostgreSQL
target_link_libraries(test PRIVATE hiredis pq curl)

add_dependencies(cc drone)

add_dependencies(test cc)

