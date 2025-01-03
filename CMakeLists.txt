cmake_minimum_required(VERSION 3.21)

project(GBEmulator)

# Enable c++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif(WIN32)

# Include RTAudio
if(UNIX)
    set(RTAUDIO_API_PULSE ON)
    set(RTAUDIO_API_WASAPI OFF)
endif(UNIX)
if(WIN32)
   set(RTAUDIO_API_WASAPI ON)
   set(RTAUDIO_API_PULSE OFF)
endif(WIN32)
set(RTAUDIO_BUILD_TESTING OFF)
add_subdirectory("${GBEmulator_SOURCE_DIR}/external/rtaudio" rtaudio)
include_directories("${GBEmulator_SOURCE_DIR}/external/rtaudio")
set_target_properties(rtaudio PROPERTIES FOLDER ${EXTERNAL_FOLDER})
if(WIN32)
  target_compile_options(rtaudio PRIVATE /wd4251 /wd4996 /wd4267)
endif(WIN32)

option(BUILD_TESTS "Build the tests" ON)

add_subdirectory("src")

if(UNIX)
  add_subdirectory("external/rgbds")
endif(UNIX)

# if (BUILD_TESTS)
    add_subdirectory("tests")
# endif(BUILD_TESTS)
