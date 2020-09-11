cmake_minimum_required(VERSION 2.8)

project(wpl.vs)

include_directories(.)

add_subdirectory(src)

if (NOT TARGET utee)
	add_subdirectory(libraries/utee)
endif()

add_subdirectory(tests)
