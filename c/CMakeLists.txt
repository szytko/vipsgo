cmake_minimum_required(VERSION 3.10)
project(ImageSDK VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)

# Find VIPS using pkg-config
pkg_check_modules(VIPS REQUIRED vips-cpp)

# Include directories
include_directories(include)

# Create shared library
add_library(vips_wrapper SHARED
    src/vips_wrapper.cpp
)

# Link libraries and set compile flags
target_link_libraries(vips_wrapper ${VIPS_LIBRARIES})
target_include_directories(vips_wrapper PRIVATE ${VIPS_INCLUDE_DIRS})
target_compile_options(vips_wrapper PRIVATE ${VIPS_CFLAGS_OTHER})
target_link_directories(vips_wrapper PRIVATE ${VIPS_LIBRARY_DIRS})

# Set optimization flags for release build
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Install targets
install(TARGETS vips_wrapper
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES include/vips_wrapper.h
    DESTINATION include
)

# Create a simple test executable
add_executable(test_wrapper test/test_wrapper.cpp)
target_link_libraries(test_wrapper vips_wrapper ${VIPS_LIBRARIES})
target_include_directories(test_wrapper PRIVATE include ${VIPS_INCLUDE_DIRS})
target_compile_options(test_wrapper PRIVATE ${VIPS_CFLAGS_OTHER})
target_link_directories(test_wrapper PRIVATE ${VIPS_LIBRARY_DIRS})

