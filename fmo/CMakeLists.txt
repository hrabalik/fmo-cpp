add_library(fmo STATIC
    "../include/fmo/assert.hpp"
    "../include/fmo/stats.hpp"
    "../include/fmo/image.hpp"
    assert.cpp
    stats.cpp
    image.cpp)

set_property(TARGET fmo PROPERTY CXX_EXTENSIONS OFF)
set_property(TARGET fmo PROPERTY CXX_STANDARD 14)

find_package(OpenCV REQUIRED core)

target_include_directories(fmo
    PRIVATE ${OpenCV_INCLUDE_DIRS}
    PUBLIC "../include")

target_link_libraries(fmo ${OpenCV_LIBS})
