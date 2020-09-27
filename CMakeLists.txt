cmake_minimum_required (VERSION 3.15)
project (cascade LANGUAGES CXX)

option (ENABLE_WERROR "Whether or not to build with warnings treated as errors" ON)
option (FMT_HEADER_ONLY "Whether or not to #define FMT_HEADER_ONLY" ON)
option (FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." ON)

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

# Enable coloring with Ninja
if (FORCE_COLORED_OUTPUT)
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")
  elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics")
  endif ()

  message (STATUS "Enabled colored output")
endif ()

# Make sure libfmt is compiled as "header-only"
if (FMT_HEADER_ONLY)
  add_definitions (-DFMT_HEADER_ONLY)
endif ()

set (LLVM_LIBS "-lLLVMCore" CACHE STRING "LLVM linker options")

file (GLOB_RECURSE SOURCE_FILES "src/*.cc" "src/**/*.cc")

# Include LLVM
find_package (LLVM REQUIRED CONFIG)
message (STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

# Include {fmt}
add_subdirectory (vendor/fmt EXCLUDE_FROM_ALL)

# Create the executable
add_executable (cascade ${SOURCE_FILES})
target_link_libraries (cascade fmt::fmt-header-only ${LLVM_LIBS})
target_include_directories (cascade PRIVATE src vendor/fmt/include vendor/cxxopts/include ${LLVM_INCLUDE_DIRS})

# Enable C++17 and disable GNU extensions
set_target_properties(cascade PROPERTIES
  CXX_STANDARD 17
  CXX_EXTENSIONS OFF
)

