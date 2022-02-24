cmake_minimum_required(VERSION 3.12)

project(GBEmulator)

# Enable c++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug" OR ${CMAKE_BUILD_TYPE} STREQUAL RelWithDebInfo)
  add_compile_definitions("_GLIBCXX_DEBUG")
endif()

set(EXTERNAL_FOLDER "External")
set(MAIN_FOLDER "GBEmulator")

if(MSVC)
  add_definitions(/MP)
endif()

# set(CMAKE_CXX_FLAGS "-Wall")
if(WIN32)
set(CMAKE_CXX_FLAGS_DEBUG "/ZI /Od")
set(CMAKE_CXX_FLAGS_RELEASE "/O2")
else(WIN32)
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
endif(WIN32)

# Generate Tonic lib
set(TONIC_DIR ${GBEmulator_SOURCE_DIR}/external/Tonic/src)
set(TONIC_INCLUDES ${TONIC_DIR} ${GBEmulator_SOURCE_DIR}/external/include)
include_directories(${TONIC_INCLUDES})
file(GLOB_RECURSE TONIC_SRC
    "${TONIC_DIR}/*"
)

add_library(Tonic STATIC ${TONIC_SRC})
if(UNIX)
  target_compile_options(Tonic PRIVATE "-Wno-implicit-int-float-conversion")
endif(UNIX)
if(WIN32)
  target_compile_options(Tonic PRIVATE /wd4068 /wd4267 /wd4244 /wd4305 /wd4018)
endif(WIN32)
set_target_properties(Tonic PROPERTIES FOLDER ${EXTERNAL_FOLDER})

# option(BUILD_TESTS "Build the tests" OFF)

add_subdirectory("src")

# if (BUILD_TESTS)
#     add_subdirectory("tests")
# endif(BUILD_TESTS)