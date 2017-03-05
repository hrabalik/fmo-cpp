find_package(OpenCV REQUIRED)

if (OpenCV_VERSION VERSION_LESS "3.0")
    find_package(OpenCV REQUIRED core highgui imgproc)
else()
    find_package(OpenCV REQUIRED core imgcodecs imgproc)
endif()

add_library(fmo STATIC
    "../include/fmo/algebra.hpp"
    "../include/fmo/assert.hpp"
    "../include/fmo/benchmark.hpp"
    "../include/fmo/common.hpp"
    "../include/fmo/explorer.hpp"
    "../include/fmo/exchange.hpp"
    "../include/fmo/image.hpp"
    "../include/fmo/processing.hpp"
    "../include/fmo/region.hpp"
    "../include/fmo/stats.hpp"
    assert.cpp
    benchmark.cpp
    explorer-components.cpp
    explorer-impl.cpp
    explorer-impl.hpp
    explorer-object.cpp
    explorer-preprocess.cpp
    explorer-strips.cpp
    explorer-trajectories.cpp
    explorer-visualize.cpp
    image.cpp
    image-util.cpp
    image-util.hpp
    include-opencv.hpp
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
install(TARGETS fmo DESTINATION lib)
