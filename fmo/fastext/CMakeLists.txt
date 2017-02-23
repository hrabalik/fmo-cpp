# remove -Werror, /WX
string (REPLACE "-Werror" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
string (REPLACE "/WX" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

add_library(fastext STATIC
	FASTex.hpp
	FASTex.cpp
	FT_common.hpp
	FT_common.cpp
	KeyPoints.h
	KeyPoints.cpp)

set_property(TARGET fastext PROPERTY CXX_EXTENSIONS OFF)
set_property(TARGET fastext PROPERTY CXX_STANDARD 14)

target_include_directories(fastext
    PRIVATE ${OpenCV_INCLUDE_DIRS})

target_link_libraries(fastext PRIVATE ${OpenCV_LIBS})
