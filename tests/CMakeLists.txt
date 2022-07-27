# Add GTest
set(BUILD_GMOCK OFF)
add_subdirectory(${GBEmulator_SOURCE_DIR}/external/googletest build)
#set_target_properties(gtest gtest_main uninstall PROPERTIES FOLDER ${EXTERNAL_FOLDER}/gtest)
enable_testing()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

include_directories(${GBEmulator_SOURCE_DIR}/include)
include_directories(${GBEmulator_SOURCE_DIR}/tests)

file(GLOB_RECURSE TESTS_SRC
    "${GBEmulator_SOURCE_DIR}/tests/*.cpp"
)

add_executable(GBEmulatorUnitTests ${TESTS_SRC})
target_link_libraries(GBEmulatorUnitTests GBEmulator_Core Tonic)
target_link_libraries(GBEmulatorUnitTests gtest_main)
set_target_properties(GBEmulatorUnitTests PROPERTIES FOLDER ${MAIN_FOLDER})

#include(GoogleTest)
#gtest_discover_tests(NesEmulatorUnitTests)