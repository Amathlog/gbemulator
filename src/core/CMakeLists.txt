add_compile_definitions(_SILENCE_CXX17_STRSTREAM_DEPRECATION_WARNING)

include_directories(${GBEmulator_SOURCE_DIR}/include)

file(GLOB_RECURSE CORELIB_SRC
    ${GBEmulator_SOURCE_DIR}/include/core/*.h
    ${GBEmulator_SOURCE_DIR}/src/core/*.cpp
)

add_library(GBEmulator_Core STATIC ${CORELIB_SRC})
target_link_libraries(GBEmulator_Core Tonic)
set_target_properties(GBEmulator_Core PROPERTIES FOLDER ${MAIN_FOLDER})