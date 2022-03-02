cmake_minimum_required(VERSION 3.10)
project("SDL2 anims")

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_C_FLAGS "-Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -D_REENTRANT")

find_package(SDL2 REQUIRED)
include_directories("${SDL2_INCLUDE_DIRS}")
include_directories("${CMAKE_SOURCE_DIR}/include")

add_executable(randwalk randwalk.c)
target_link_libraries(randwalk ${SDL2_LIBRARIES})

add_executable(spect-vis "spect-vis.c")
target_link_libraries(spect-vis ${SDL2_LIBRARIES} m)


add_executable(drawpad drawpad.c)
target_link_libraries(drawpad ${SDL2_LIBRARIES})

