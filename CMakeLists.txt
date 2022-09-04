cmake_minimum_required(VERSION 3.13)

project(wpl.vs)

enable_testing()
cmake_policy(SET CMP0077 NEW)
cmake_policy(SET CMP0079 NEW)

option(WPL_VS_NO_TESTS "Do not build test modules." OFF)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/make/build.props)

include(unipath)

if (NOT TARGET freetype)
	add_subdirectory(libraries/freetype2)
endif()

if (NOT TARGET utfia)
	set(UTFIA_NO_TESTS ON)
	add_subdirectory(libraries/utfia)
endif()

if (NOT TARGET agge)
	set(AGGE_NO_TESTS ON)
	set(AGGE_NO_EXTENSIONS ON)
	add_subdirectory(libraries/agge)
endif()

if (NOT TARGET wpl)
	set(WPL_NO_TESTS ON)
	add_subdirectory(libraries/wpl)
endif()

add_subdirectory(src)

if (NOT WPL_VS_NO_TESTS)
	if (NOT TARGET utee)
		set(UTEE_NO_TESTS ON)
		add_subdirectory(libraries/utee)
	endif()

	add_subdirectory(tests)
	add_utee_test(wpl.vs.tests)
endif()
