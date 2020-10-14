cmake_minimum_required(VERSION 3.10)

# Set the project name
project(eva VERSION 1.0 LANGUAGES CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB SOURCES
	source/**
	source/EVA/**
	source/EVA/*/**
)

# Add the library
add_library(eva STATIC ${SOURCES})

# GLFW
find_package(OpenGL REQUIRED)
list(APPEND EXTRA_INCLUDES ${OPENGL_INCLUDE_DIRS})

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set( GLFW_BUILD_INSTALL OFF CACHE BOOL  "GLFW lib only" )
add_subdirectory(dependencies/glfw)

if(WIN32)
    SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup" )
endif()

add_compile_definitions(GLFW_INCLUDE_NONE)
target_link_libraries(eva PUBLIC glfw)

# GLAD
add_subdirectory(dependencies/glad)
target_link_libraries(eva PUBLIC glad)

# GLM
add_subdirectory(dependencies/glm)
target_link_libraries(eva PUBLIC glm)

# SpdLog
add_subdirectory(dependencies/spdlog)
target_link_libraries(eva PUBLIC spdlog)

# IMGUI
add_compile_definitions(IMGUI_IMPL_OPENGL_LOADER_GLAD)
FILE(GLOB IMGUI_SOURCES 
	"${CMAKE_CURRENT_SOURCE_DIR}/dependencies/imgui/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/dependencies/imgui/examples/imgui_impl_opengl3.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/dependencies/imgui/examples/imgui_impl_glfw.cpp")
list(APPEND EXTRA_SOURCES ${IMGUI_SOURCES})
list(APPEND EXTRA_INCLUDES "dependencies/imgui")

# stb
FILE(GLOB STB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/stb/*.cpp")
list(APPEND EXTRA_SOURCES ${STB_SOURCES})
list(APPEND EXTRA_INCLUDES "dependencies/stb")

# Platforms
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	add_compile_definitions(EVA_PLATFORM_WINDOWS)
	add_compile_definitions(EVA_PLATFORM="windows")

	FILE(GLOB WINDOWS_SOURCES "source/Platform/Windows/**")
	list(APPEND EXTRA_SOURCES ${WINDOWS_SOURCES})
	list(APPEND EXTRA_INCLUDES PRIVATE "source/Platform/Windows")

elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_compile_definitions(EVA_PLATFORM_LINUX)
	add_compile_definitions(EVA_PLATFORM="linux")
endif()

# Render API
FILE(GLOB OPENGL_SOURCES "source/Platform/OpenGL/**")
list(APPEND EXTRA_SOURCES ${OPENGL_SOURCES})

# Include paths
target_include_directories(eva 
	PUBLIC "source"
	PUBLIC ${EXTRA_INCLUDES}
)

# Target
target_sources(eva PRIVATE ${EXTRA_SOURCES})
target_precompile_headers(eva PUBLIC "source/pch.hpp")

# Test
add_executable(eva_test "test/source/main.cpp")
target_precompile_headers(eva_test PUBLIC "source/pch.hpp")
target_link_libraries(eva_test PRIVATE eva)

# Definitions
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_compile_definitions(EVA_ARCHITECTURE_64)
	add_compile_definitions(EVA_ARCHITECTURE="x64")
else()
    add_compile_definitions(EVA_ARCHITECTURE_32)
	add_compile_definitions(EVA_ARCHITECTURE="x86")
endif()

if(CMAKE_BUILD_TYPE MATCHES "Debug")
	add_compile_definitions(EVA_DEBUG)
	add_compile_definitions(EVA_CONFIGURATION="debug")
elseif (CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
	add_compile_definitions(EVA_RELEASE)
	add_compile_definitions(EVA_CONFIGURATION="release")
endif()
