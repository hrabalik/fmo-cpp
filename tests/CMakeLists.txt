# fmo-test

add_executable(fmo-test
    ../catch/catch.hpp
	test-benchmark.cpp
    test-explorer.cpp
    test-image.cpp
    test-main.cpp)

set_property(TARGET fmo-test PROPERTY CXX_EXTENSIONS OFF)
set_property(TARGET fmo-test PROPERTY CXX_STANDARD 14)

target_link_libraries(fmo-test fmo)

set(ASSETS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/assets")
set(ASSETS_DEST1 "${CMAKE_CURRENT_BINARY_DIR}/assets")
set(ASSETS_DEST2 "$<TARGET_FILE_DIR:fmo-test>/assets")

add_custom_command(TARGET fmo-test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${ASSETS_SRC} ${ASSETS_DEST1})

add_custom_command(TARGET fmo-test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${ASSETS_SRC} ${ASSETS_DEST2})

# fmo-benchmark

add_executable(fmo-benchmark
	benchmark-main.cpp)

set_property(TARGET fmo-benchmark PROPERTY CXX_EXTENSIONS OFF)
set_property(TARGET fmo-benchmark PROPERTY CXX_STANDARD 14)

target_link_libraries(fmo-benchmark fmo)
