find_package(OpenCV REQUIRED core imgcodecs imgproc)

add_subdirectory("fastext" "")

add_library(fmo STATIC
    "../include/fmo/assert.hpp"
    "../include/fmo/benchmark.hpp"
    "../include/fmo/common.hpp"
    "../include/fmo/exchange.hpp"
    "../include/fmo/image.hpp"
    "../include/fmo/processing.hpp"
    "../include/fmo/region.hpp"
    "../include/fmo/stats.hpp"
    assert.cpp
	benchmark.cpp
    image.cpp
    image-util.cpp
    image-util.hpp
    processing-basic.cpp
    processing-complex.cpp
    region.cpp
    stats.cpp)

set_property(TARGET fmo PROPERTY CXX_EXTENSIONS OFF)
set_property(TARGET fmo PROPERTY CXX_STANDARD 14)

target_include_directories(fmo
    PRIVATE ${OpenCV_INCLUDE_DIRS}
    PUBLIC "../include")

target_link_libraries(fmo PRIVATE ${OpenCV_LIBS})
