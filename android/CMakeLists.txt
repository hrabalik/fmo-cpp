add_library(fmo-android SHARED
    java_interface.hpp
    java_interface.cpp
    java_classes.hpp
    java_classes.cpp
    recording2.cpp)

set_property(TARGET fmo-android PROPERTY CXX_EXTENSIONS OFF)
set_property(TARGET fmo-android PROPERTY CXX_STANDARD 14)

target_link_libraries(fmo-android fmo)
