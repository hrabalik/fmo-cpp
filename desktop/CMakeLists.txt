add_executable(fmo-desktop
    main.cpp)

set_property(TARGET fmo-desktop PROPERTY CXX_EXTENSIONS OFF)
set_property(TARGET fmo-desktop PROPERTY CXX_STANDARD 14)

target_link_libraries(fmo-desktop fmo)
