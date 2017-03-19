# fmo-core

add_library(fmo-core STATIC
    "../include/fmo/algebra.hpp"
    "../include/fmo/algorithm.hpp"
    "../include/fmo/assert.hpp"
    "../include/fmo/benchmark.hpp"
    "../include/fmo/common.hpp"
    "../include/fmo/exchange.hpp"
    "../include/fmo/image.hpp"
    "../include/fmo/pointset.hpp"
    "../include/fmo/processing.hpp"
    "../include/fmo/region.hpp"
    "../include/fmo/stats.hpp"
    algorithm.cpp
    assert.cpp
    benchmark.cpp
    image.cpp
    image-util.cpp
    image-util.hpp
    include-opencv.hpp
    processing-basic.cpp
    processing-complex.cpp
    region.cpp
    stats.cpp
)

set_property(TARGET fmo-core PROPERTY CXX_EXTENSIONS OFF)
set_property(TARGET fmo-core PROPERTY CXX_STANDARD 14)

target_include_directories(fmo-core
    PRIVATE ${OpenCV_INCLUDE_DIRS}
    PUBLIC "../include")

target_link_libraries(fmo-core PRIVATE ${OpenCV_LIBS})

# subdirectories

add_subdirectory("explorer-v1" "")

# fmo

set(FMO_LIBS "fmo-core;fmo-explorer-v1" CACHE INTERNAL "List of FMO libraries to link")
