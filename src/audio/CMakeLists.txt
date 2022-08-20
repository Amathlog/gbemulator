add_compile_definitions(_SILENCE_CXX17_STRSTREAM_DEPRECATION_WARNING)

include_directories(${GBEmulator_SOURCE_DIR}/include)

file(GLOB_RECURSE AUDIOLIB_SRC
    ${GBEmulator_SOURCE_DIR}/include/audio/*.h
    ${GBEmulator_SOURCE_DIR}/src/audio/*.cpp
)

# Include RTAudio
include_directories("${GBEmulator_SOURCE_DIR}/external/rtaudio")

add_executable(GBEmulator_Audio ${AUDIOLIB_SRC})
target_link_libraries(GBEmulator_Audio rtaudio)
set_target_properties(GBEmulator_Audio PROPERTIES FOLDER ${MAIN_FOLDER})
