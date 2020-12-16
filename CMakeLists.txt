cmake_minimum_required(VERSION 3.10)

# Set the project name
project(eva VERSION 1.0 LANGUAGES CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

###################################### EVA #####################################

file(GLOB_RECURSE SOURCES
	source/EVA/*
)
add_library(eva STATIC ${SOURCES})

#OpenGL
find_package(OpenGL REQUIRED)
list(APPEND EXTRA_INCLUDES ${OPENGL_INCLUDE_DIRS})
FILE(GLOB OPENGL_SOURCES "source/Platform/OpenGL/**")
list(APPEND EXTRA_SOURCES ${OPENGL_SOURCES})

# GLFW
add_subdirectory(dependencies/glfw)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set( GLFW_BUILD_INSTALL OFF CACHE BOOL  "GLFW lib only" )

if(WIN32)
    SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup" )
endif()

add_compile_definitions(GLFW_INCLUDE_NONE)
target_link_libraries(eva PUBLIC glfw ${GLFW_LIBRARIES})

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
add_subdirectory(dependencies/imgui)
target_link_libraries(eva PUBLIC imgui)

# stb
add_subdirectory(dependencies/stb)
target_link_libraries(eva PUBLIC stb)

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

	FILE(GLOB WINDOWS_SOURCES "source/Platform/Linux/**")
	list(APPEND EXTRA_SOURCES ${WINDOWS_SOURCES})
	list(APPEND EXTRA_INCLUDES PRIVATE "source/Platform/Linux")
endif()

# Include paths
target_include_directories(eva 
	PUBLIC "source"
	PUBLIC ${EXTRA_INCLUDES}
)

# Target
target_sources(eva PRIVATE ${EXTRA_SOURCES})
target_precompile_headers(eva PUBLIC "source/pch.hpp")

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

##################################### Test #####################################

add_executable(eva_test "test/source/main.cpp")

set_target_properties(
    eva_test
    PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_link_libraries(eva_test PRIVATE eva)

# Copy assets
file(REMOVE_RECURSE ${PROJECT_BINARY_DIR}/bin/assets)
file(COPY  ${PROJECT_SOURCE_DIR}/test/assets DESTINATION ${PROJECT_BINARY_DIR}/bin)
