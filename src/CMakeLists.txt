cmake_minimum_required(VERSION 3.13)

include(vssdk)

add_definitions(-D_ATL_FREE_THREADED)

set(WPL_VS_SOURCES
	async.cpp
	factory.cpp
	frame.cpp
	ole-command-target.cpp
	package.cpp
	pane.cpp
)

add_library(wpl.vs STATIC ${WPL_VS_SOURCES})

target_link_libraries(wpl.vs wpl)
target_include_directories(wpl.vs PUBLIC .. PRIVATE fallback)
