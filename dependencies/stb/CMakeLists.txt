cmake_minimum_required(VERSION 3.10)

# Set the project name
project(stb VERSION 1.0 LANGUAGES CXX)

file(GLOB SOURCES
	source/**
)

add_library(stb STATIC ${SOURCES})

# Include paths
target_include_directories(stb 
	PUBLIC "source"
)