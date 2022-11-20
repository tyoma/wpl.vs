cmake_minimum_required(VERSION 3.13)

include(vssdk)

set(MP_INTDIR ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR})
set(MP_OUTDIR $<TARGET_FILE_DIR:styles-discover>)

include_directories(.)
include_directories(${MP_INTDIR})

add_custom_command(OUTPUT ${MP_INTDIR}/commands.cto
	COMMAND "${VSSDKBIN}/vsct.exe" "${CMAKE_CURRENT_SOURCE_DIR}/resources/commands.vsct" "${MP_INTDIR}/commands.cto" -I"${VSSDKINCLUDE}" -DMP_MSVS11PLUS_SUPPORT
	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/resources/commands.vsct"
	COMMENT "Compiling vsct..."
)

add_custom_command(OUTPUT ${MP_INTDIR}/commands.vs10.cto
	COMMAND "${VSSDKBIN}/vsct.exe" "${CMAKE_CURRENT_SOURCE_DIR}/resources/commands.vsct" "${MP_INTDIR}/commands.vs10.cto" -I"${VSSDKINCLUDE}"
	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/resources/commands.vsct"
	COMMENT "Compiling vsct..."
)

add_custom_command(OUTPUT package-metadata
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/styles-discover.pkgdef ${MP_OUTDIR}
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/extension.vsixmanifest ${MP_OUTDIR}
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/extension.vs10.vsixmanifest ${MP_OUTDIR}
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/styles-discover.pkgdef
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/extension.vsixmanifest
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/extension.vs10.vsixmanifest
	COMMENT "Copying Metadata..."
)

set(STYLES_DISCOVER_SOURCES
	main.cpp
	models.cpp
	styles-discover_package.cpp
	styles-discover.def
	
	package-metadata
	${MP_INTDIR}/commands.cto
)

set(STYLES_DISCOVER_UI_SOURCES
	resources/styles-discover_ui.rc

	${MP_INTDIR}/commands.cto
)

set(STYLES_DISCOVER_UI_VS10_SOURCES
	resources/styles-discover_ui.rc

	${MP_INTDIR}/commands.vs10.cto
)

add_library(styles-discover SHARED ${STYLES_DISCOVER_SOURCES})
add_library(styles-discover.ui SHARED ${STYLES_DISCOVER_UI_SOURCES})
add_library(styles-discover.ui.vs10 SHARED ${STYLES_DISCOVER_UI_VS10_SOURCES})

target_link_libraries(styles-discover wpl.vs samples.common wpl)

set_target_properties(styles-discover.ui styles-discover.ui.vs10 PROPERTIES LINK_OPTIONS /NOENTRY RUNTIME_OUTPUT_DIRECTORY ${MP_OUTDIR}/1033)
foreach(CFG ${CMAKE_CONFIGURATION_TYPES})
	string(TOUPPER ${CFG} CFG)
	set_target_properties(styles-discover.ui styles-discover.ui.vs10 PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${CFG} ${MP_OUTDIR}/1033)
endforeach()
