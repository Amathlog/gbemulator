add_subdirectory(core)
add_subdirectory(libretro_core)
#add_subdirectory(audio)

option(GBEMULATOR_ONLY_CORE "Build only the core lib" OFF)
option(GBEMULATOR_USE_SDL "Use SDL and not home renderer." ON)

if (NOT GBEMULATOR_ONLY_CORE)
    add_subdirectory(exe)
endif()
