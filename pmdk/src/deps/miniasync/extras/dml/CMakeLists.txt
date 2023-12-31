#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021-2022, Intel Corporation
#

project(miniasync-vdm-dml C)

add_cstyle(miniasync-vdm-dml
	${CMAKE_CURRENT_SOURCE_DIR}/*.[ch]
	${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/libminiasync-vdm-dml/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/utils/*.[ch])

add_check_whitespace(miniasync-vdm-dml
	${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
	${CMAKE_CURRENT_SOURCE_DIR}/README.md
	${CMAKE_CURRENT_SOURCE_DIR}/*.[ch]
	${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/libminiasync-vdm-dml/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/utils/*.[ch])

set(DML_SOURCES
	data_mover_dml.c
)

add_library(miniasync-vdm-dml SHARED ${DML_SOURCES})
target_link_libraries(miniasync-vdm-dml PRIVATE dml dl)

target_include_directories(miniasync-vdm-dml PRIVATE .
	${MINIASYNC_DML_INCLUDE_DIR}
	${MINIASYNC_INCLUDE_DIR}
	${MINIASYNC_SOURCE_DIR})

set_target_properties(miniasync-vdm-dml PROPERTIES
	PUBLIC_HEADER ${MINIASYNC_DML_INCLUDE_DIR}/libminiasync-vdm-dml.h
)

install(TARGETS miniasync-vdm-dml
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Install headers included in public header
install(DIRECTORY ${MINIASYNC_DML_INCLUDE_DIR}/libminiasync-vdm-dml
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	FILES_MATCHING PATTERN "*.h"
)

set(CHECK_MOVDIR64B_SOURCES
	${MINIASYNC_DML_SOURCE_DIR}/utils/check_movdir64b.c
	${CORE_SOURCE_DIR}/cpu.c)

add_link_executable(check_movdir64b
	"${CHECK_MOVDIR64B_SOURCES}"
	"dml")
