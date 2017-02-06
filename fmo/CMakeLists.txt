add_library(fmo STATIC
    "../include/fmo/assert.hpp"
	"../include/fmo/stats.hpp"
    assert.cpp
	stats.cpp)

set_property(TARGET fmo PROPERTY CXX_EXTENSIONS OFF)

target_compile_features(fmo
    PUBLIC
        cxx_constexpr
		cxx_defaulted_functions
		cxx_deleted_functions
		cxx_generalized_initializers
		cxx_inheriting_constructors
		cxx_noexcept
		cxx_nonstatic_member_init
		cxx_nullptr
		cxx_static_assert
		cxx_rvalue_references
		cxx_strong_enums
		cxx_variadic_templates
		cxx_decltype_auto
    PRIVATE
	    cxx_inline_namespaces
		cxx_lambdas
		cxx_range_for
		cxx_thread_local
		cxx_auto_type
		cxx_generic_lambdas)

find_package(OpenCV REQUIRED core)

target_include_directories(fmo
    PRIVATE ${OpenCV_INCLUDE_DIRS}
    PUBLIC "../include")

target_link_libraries(fmo ${OpenCV_LIBS})
