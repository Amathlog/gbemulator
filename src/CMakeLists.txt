add_subdirectory(core)

option(GBEMULATOR_ONLY_CORE "Build only the core lib" OFF)

if (NOT GBEMULATOR_ONLY_CORE)
    add_subdirectory(exe)
endif(NOT GBEMULATOR_ONLY_CORE)