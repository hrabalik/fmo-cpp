add_library(fmo-android SHARED
    ../catch/catch.hpp
    java_interface.hpp
    java_interface.cpp
    java_classes.hpp
    java_classes.cpp
    benchmark.hpp
    benchmark.cpp
    benchmark_runner.cpp
    recording.cpp)

set_property(TARGET fmo-android PROPERTY CXX_EXTENSIONS OFF)
set_property(TARGET fmo-android PROPERTY CXX_STANDARD 14)

target_link_libraries(fmo-android fmo)
