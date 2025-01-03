# Include OpenGL
if(UNIX)
    set(OpenGL_GL_PREFERENCE "GLVND")
endif(UNIX)

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

# Include GLFW and GLAD
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

set(GLFW_DIR "${GBEmulator_SOURCE_DIR}/external/glfw")
set(GLAD_DIR "${GBEmulator_SOURCE_DIR}/external/glad")
add_subdirectory("${GLFW_DIR}" glfw)
include_directories("${GLFW_DIR}/include")
include_directories("${GLAD_DIR}/include")

# Build ImGui
set(IMGUI_DIR "${GBEmulator_SOURCE_DIR}/external/imgui")
include_directories("${IMGUI_DIR}")
include_directories("${IMGUI_DIR}/backends")

file (GLOB IMGUI_SRC
    "${IMGUI_DIR}/*cpp"
    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)

add_library(ImGui STATIC ${IMGUI_SRC})
set_target_properties(ImGui PROPERTIES FOLDER ${EXTERNAL_FOLDER})

# Build ImGuiFileBrowser
set(IMGUI_FE_DIR "${GBEmulator_SOURCE_DIR}/external/ImGui-Addons/FileBrowser")
include_directories("${IMGUI_FE_DIR}")
file (GLOB IMGUI_FE_SRC
    "${IMGUI_FE_DIR}/*.cpp"
)

add_library(ImGuiFE STATIC ${IMGUI_FE_SRC})
set_target_properties(ImGuiFE PROPERTIES FOLDER ${EXTERNAL_FOLDER})
target_link_libraries(ImGuiFE ImGui)
if(WIN32)
  target_compile_options(ImGuiFE PRIVATE /wd4267 /wd4996 /wd4244)
endif(WIN32)

include_directories(${GBEmulator_SOURCE_DIR}/include)

file(GLOB_RECURSE NEW_EXE_SRC
    "${GBEmulator_SOURCE_DIR}/src/exe/*.cpp"
    "${GBEmulator_SOURCE_DIR}/include/exe/*.h"
    "${GLAD_DIR}/src/glad.c"
)

add_executable(GBEmulator ${NEW_EXE_SRC})
target_link_libraries(GBEmulator GBEmulator_Core ${OPENGL_LIBRARIES} glfw ${CMAKE_DL_LIBS} rtaudio ImGuiFE ImGui)
set_target_properties(GBEmulator PROPERTIES FOLDER ${MAIN_FOLDER})

install(FILES $<TARGET_RUNTIME_DLLS:GBEmulator> TYPE BIN)