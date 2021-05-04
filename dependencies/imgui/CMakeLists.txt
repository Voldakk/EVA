cmake_minimum_required(VERSION 3.10)

# Imgui
project(imgui VERSION 1.0 LANGUAGES CXX)
file(GLOB SOURCES
	"imgui/*.cpp"
	"imgui/backends/imgui_impl_opengl3.cpp"
	"imgui/backends/imgui_impl_glfw.cpp"

	"imgui-node-editor/*.cpp"

	"src/*.cpp"

)
add_compile_definitions(IMGUI_IMPL_OPENGL_LOADER_GLAD)
add_library(imgui STATIC ${SOURCES})
target_link_libraries(imgui PRIVATE glad glfw ${GLFW_LIBRARIES})
target_include_directories(imgui 
	PUBLIC "imgui/" "imgui-node-editor/" "src/"
)
