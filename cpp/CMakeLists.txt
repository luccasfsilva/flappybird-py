cmake_minimum_required(VERSION 3.10)
project(flappy_bird CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

find_package(SDL2 REQUIRED)

add_executable(flappy main.cpp)

if(TARGET SDL2::SDL2)
  target_link_libraries(flappy PRIVATE SDL2::SDL2)
  if(TARGET SDL2::SDL2main)
    target_link_libraries(flappy PRIVATE SDL2::SDL2main)
  endif()
else()
  target_include_directories(flappy PRIVATE ${SDL2_INCLUDE_DIRS})
  target_link_libraries(flappy PRIVATE ${SDL2_LIBRARIES})
endif()

if(MSVC)
  target_compile_options(flappy PRIVATE /W4)
else()
  target_compile_options(flappy PRIVATE -Wall -Wextra)
endif()
