cmake_minimum_required(VERSION 3.4.1)

find_package(OpenCV REQUIRED core)
include_directories(${OpenCV_INCLUDE_DIRS})

add_library(fmo-android SHARED
    assert.hpp
    assert.cpp
    stats.hpp
    stats.cpp
    java_interface.hpp
    java_interface.cpp
    java_classes.hpp
    java_classes.cpp
    ocvrec2.cpp
    recording2.cpp)
target_link_libraries(fmo-android ${OpenCV_LIBS})
