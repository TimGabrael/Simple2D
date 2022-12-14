cmake_minimum_required(VERSION 3.8)
project(Simple2Dproj VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(3rd-party/glfw)
add_subdirectory(3rd-party/glad)
add_subdirectory(3rd-party/NFDriver)
add_subdirectory(3rd-party/box2d/src)
add_subdirectory(3rd-party/zlib)


set(IMGUI_FILES "3rd-party/imgui/imgui.cpp" "3rd-party/imgui/imgui_demo.cpp" "3rd-party/imgui/imgui_draw.cpp" "3rd-party/imgui/imgui_tables.cpp" "3rd-party/imgui/imgui_widgets.cpp" "3rd-party/imgui/backends/imgui_impl_glfw.cpp" "3rd-party/imgui/backends/imgui_impl_opengl3.cpp")

set(ENGINE_FILES ${IMGUI_FILES} "src/GameState.cpp" "src/Audio/AudioManager.cpp" "src/Audio/WavFile.cpp" "src/Graphics/Scene.cpp" "src/Util/Assets.cpp" "src/Physics/Physics.cpp" "src/Graphics/Renderer.cpp" "src/Graphics/Renderables.cpp")
set(ENGINE_LIBRARIES ${GLAD_LIBRARIES} glfw NFDriver zlibstatic Xinput box2d)
set(ENGINE_INCLUDES 3rd-party/glfw/include 3rd-party/glm 3rd-party/NFDriver/include 3rd-party/rectpack2D/src 3rd-party/imgui 3rd-party/imgui/backends 3rd-party/box2d/include src/zlibconfig 3rd-party/zlib/ src/)

list(TRANSFORM ENGINE_FILES PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)
list(TRANSFORM ENGINE_INCLUDES PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)

set(ENGINE_FILES ${ENGINE_FILES} PARENT_SCOPE)
set(ENGINE_LIBRARIES ${ENGINE_LIBRARIES} PARENT_SCOPE)
set(ENGINE_INCLUDES ${ENGINE_INCLUDES} PARENT_SCOPE)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	add_executable(Example2D "example/main.cpp" ${ENGINE_FILES} "example/Entitys.h" "example/Entitys.cpp" "example/GameManager.cpp")
	set_target_properties(Example2D PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
	target_include_directories(Example2D PRIVATE ${ENGINE_INCLUDES})
	target_link_libraries(Example2D PRIVATE ${ENGINE_LIBRARIES})
endif()