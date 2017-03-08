find_package(OpenCV REQUIRED)

if (OpenCV_VERSION VERSION_LESS "3.0")
    find_package(OpenCV REQUIRED core highgui imgproc)
else()
    find_package(OpenCV REQUIRED core highgui imgproc videoio)
endif()

add_executable(fmo-desktop
    args.cpp
    args.hpp
    config.cpp
    config.hpp
    desktop-main.cpp
    desktop-opencv.hpp
    evaluator.cpp
    evaluator.hpp
    param.cpp
    param.hpp
    parser.cpp
    parser.hpp)

set_property(TARGET fmo-desktop PROPERTY CXX_EXTENSIONS OFF)
set_property(TARGET fmo-desktop PROPERTY CXX_STANDARD 14)

target_compile_definitions(fmo-desktop PRIVATE FMO_BINARY_NAME=fmo-desktop)
target_include_directories(fmo-desktop PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(fmo-desktop PRIVATE fmo ${OpenCV_LIBS})
install(TARGETS fmo-desktop DESTINATION bin)
