cmake_minimum_required(VERSION 2.8)

include(test)

set(WPL_VS_TESTS_SOURCES
	CommandTargetTests.cpp
)

add_library(wpl.vs.tests SHARED ${WPL_VS_TESTS_SOURCES})

target_link_libraries(wpl.vs.tests wpl.vs)
