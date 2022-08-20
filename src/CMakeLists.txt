add_subdirectory(core)
add_subdirectory(audio)

option(GBEMULATOR_ONLY_CORE "Build only the core lib" OFF)

if (NOT GBEMULATOR_ONLY_CORE)
    add_subdirectory(exe)
endif(NOT GBEMULATOR_ONLY_CORE)
