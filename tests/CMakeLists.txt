cmake_minimum_required(VERSION 3.13)

set(WPL_VS_TESTS_SOURCES
	CommandTargetTests.cpp
)

add_library(wpl.vs.tests SHARED ${WPL_VS_TESTS_SOURCES})

target_link_libraries(wpl.vs.tests wpl.vs)
