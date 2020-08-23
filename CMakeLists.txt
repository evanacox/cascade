cmake_minimum_required (VERSION 3.15)
project (cascade LANGUAGES CXX)

option (ENABLE_WERROR "Whether or not to build with warnings treated as errors" ON)
option (FMT_HEADER_ONLY "Whether or not to #define FMT_HEADER_ONLY" ON)

file (GLOB_RECURSE SOURCE_FILES "src/*.cc" "src/**/*.cc")

# Include LLVM
find_package (LLVM REQUIRED CONFIG)
message (STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
llvm_map_components_to_libnames (LLVM_LIBS support core irreader target)

# Include {fmt}
add_subdirectory (vendor/fmt EXCLUDE_FROM_ALL)

# Create the executable
add_executable (cascade ${SOURCE_FILES})
target_link_libraries (cascade fmt::fmt-header-only ${LLVM_LIBS})
target_include_directories (cascade PRIVATE include vendor/fmt/include vendor/cxxopts/include ${LLVM_INCLUDE_DIRS})

# Enable C++20 for Concepts / other addons
set_target_properties(cascade PROPERTIES
    CXX_STANDARD 17
    CXX_EXTENSIONS OFF
)

# enable all warnings and -Werror 
if (ENABLE_WERROR)
    if (MSVC)
        set (WARNING_OPTIONS "/W4 /WX")
    else ()
        set (WARNING_OPTIONS "-Wall -Wextra -pedantic -Wimplicit-fallthrough -Wshadow -Werror")
    endif ()

    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_OPTIONS}")
    message (STATUS "Current C++ flags: ${CMAKE_CXX_FLAGS}")
endif ()

if (FMT_HEADER_ONLY)
    add_definitions (-DFMT_HEADER_ONLY)
endif ()