add_executable(fmo-desktop
    args.cpp
    args.hpp
    calendar.cpp
    calendar.hpp
    desktop-opencv.hpp
    evaluator.cpp
    evaluator.hpp
    frameset.cpp
    frameset.hpp
    loop.hpp
    loop-main.cpp
    loop-process.cpp
    loop-visualizer.cpp
    parser.cpp
    parser.hpp
    report.cpp
    report.hpp
    recorder.cpp
    recorder.hpp
    video.cpp
    video.hpp
    window.cpp
    window.hpp)

set_property(TARGET fmo-desktop PROPERTY CXX_EXTENSIONS OFF)
set_property(TARGET fmo-desktop PROPERTY CXX_STANDARD 14)

target_compile_definitions(fmo-desktop PRIVATE FMO_BINARY_NAME=fmo-desktop)
target_include_directories(fmo-desktop PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(fmo-desktop PRIVATE ${FMO_LIBS} ${OpenCV_LIBS} Threads::Threads)
install(TARGETS fmo-desktop DESTINATION bin)
