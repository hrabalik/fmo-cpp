add_executable(fmo-desktop
    config.cpp
    config.hpp
    desktop-main.cpp
    param.cpp
    param.hpp)

set_property(TARGET fmo-desktop PROPERTY CXX_EXTENSIONS OFF)
set_property(TARGET fmo-desktop PROPERTY CXX_STANDARD 14)

target_compile_definitions(fmo-desktop PRIVATE FMO_BINARY_NAME=fmo-desktop)
target_link_libraries(fmo-desktop fmo)
install(TARGETS fmo-desktop DESTINATION bin)
