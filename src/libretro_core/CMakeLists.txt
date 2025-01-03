include_directories(${GBEmulator_SOURCE_DIR}/include)
include_directories(${GBEmulator_SOURCE_DIR}/external/libretro-common/include)


file(GLOB_RECURSE GBRETROCORELIB_SRC
    ${GBEmulator_SOURCE_DIR}/src/libretro_core/*.cpp
)

add_library(GBEmulator_LibretroCore SHARED ${GBRETROCORELIB_SRC})
target_link_libraries(GBEmulator_LibretroCore GBEmulator_Core)
set_target_properties(GBEmulator_LibretroCore PROPERTIES FOLDER ${MAIN_FOLDER})