add_library(fmo-android SHARED
    java_interface.hpp
    java_interface.cpp
    java_classes.hpp
    java_classes.cpp
    ocvrec2.cpp
    recording2.cpp)

target_link_libraries(fmo-android fmo)
