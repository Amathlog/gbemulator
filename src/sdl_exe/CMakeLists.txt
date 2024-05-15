# Include OpenGL
if(UNIX)
    set(OpenGL_GL_PREFERENCE "GLVND")
endif(UNIX)

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

# # Include GLFW and GLAD
# set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
# set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
# set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
# set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# set(GLFW_DIR "${GBEmulator_SOURCE_DIR}/external/glfw")
# set(GLAD_DIR "${GBEmulator_SOURCE_DIR}/external/glad")
# add_subdirectory("${GLFW_DIR}" glfw)
# include_directories("${GLFW_DIR}/include")
# include_directories("${GLAD_DIR}/include")

# Build ImGui
set(IMGUI_DIR "${GBEmulator_SOURCE_DIR}/external/imgui")
include_directories("${IMGUI_DIR}")
include_directories("${IMGUI_DIR}/backends")

file (GLOB IMGUI_SRC
    "${IMGUI_DIR}/*cpp"
    "${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)

add_library(ImGui STATIC ${IMGUI_SRC})
set_target_properties(ImGui PROPERTIES FOLDER ${EXTERNAL_FOLDER})

# # Build ImGuiFileBrowser
# set(IMGUI_FE_DIR "${GBEmulator_SOURCE_DIR}/external/ImGui-Addons/FileBrowser")
# include_directories("${IMGUI_FE_DIR}")
# file (GLOB IMGUI_FE_SRC
#     "${IMGUI_FE_DIR}/*.cpp"
# )

# add_library(ImGuiFE STATIC ${IMGUI_FE_SRC})
# set_target_properties(ImGuiFE PROPERTIES FOLDER ${EXTERNAL_FOLDER})
# target_link_libraries(ImGuiFE ImGui)
# if(WIN32)
#   target_compile_options(ImGuiFE PRIVATE /wd4267 /wd4996 /wd4244)
# endif(WIN32)

include_directories(${GBEmulator_SOURCE_DIR}/include)

# Build SDL
set(SDL_DIR "${GBEmulator_SOURCE_DIR}/external/SDL")
add_subdirectory(${SDL_DIR} SDL)

include_directories("${SDL_DIR}/include")

file(GLOB_RECURSE SDL_EXE_SRC
    "${GBEmulator_SOURCE_DIR}/src/sdl_exe/*.cpp"
    "${GBEmulator_SOURCE_DIR}/include/sdl_exe/*.h"
)

add_executable(GBEmulator_SDL ${SDL_EXE_SRC})
target_link_libraries(GBEmulator_SDL GBEmulator_Core ${OPENGL_LIBRARIES} ImGui SDL3-static)
set_target_properties(GBEmulator_SDL PROPERTIES FOLDER ${MAIN_FOLDER})
