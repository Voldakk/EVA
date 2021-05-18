cmake_minimum_required(VERSION 3.10)

# Set the project name
project(glad VERSION 1.0 LANGUAGES C)

file(GLOB SOURCES
	src/**
)

add_library(glad STATIC ${SOURCES})

# Include paths
target_include_directories(glad
	PUBLIC "include"
)