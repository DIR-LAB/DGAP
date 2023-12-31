# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021-2022, Intel Corporation

#
# doc/CMakeLists.txt - prepares 'make doc' command for documentation
#

# Converts md files into manpage format. Requires pandoc command.
# NAME is output manpage name, INPUT is a path to the source md file
function(configure_man NAME INPUT)
	add_custom_command(OUTPUT ${MAN_DIR}/${NAME}
		MAIN_DEPENDENCY ${INPUT}
		COMMAND ${MINIASYNC_ROOT_DIR}/utils/md2man/md2man.sh
			${INPUT}
			${MINIASYNC_ROOT_DIR}/utils/md2man/default.man
			${MAN_DIR}/${NAME}
			${VERSION})
	list(APPEND MANPAGE_OUTFILES ${MAN_DIR}/${NAME})
	set(MANPAGE_OUTFILES ${MANPAGE_OUTFILES} PARENT_SCOPE)
endfunction()

# Generate files (to be installed) - links of C API functions (passed as {ARGN})
# to chosen (3) manpage.
function(add_manpage_links MANPAGE)
	foreach(function ${ARGN})
		set(CONTENT ".so ${MANPAGE}")
		file(WRITE ${MAN_DIR}/${function}.3 "${CONTENT}")
	endforeach()
endfunction()

# ----------------------------------------------------------------- #
## Setup custom targets and useful variables
# ----------------------------------------------------------------- #
set(MANPAGE_OUTFILES "")
set(MAN_DIR ${CMAKE_CURRENT_BINARY_DIR}/man)
add_check_whitespace(man ${CMAKE_CURRENT_SOURCE_DIR}/*.*)

# ----------------------------------------------------------------- #
## Prepare C documentation (using manpage format)
# ----------------------------------------------------------------- #
find_program(PANDOC NAMES pandoc)
if(PANDOC)
	message(STATUS "Found pandoc: ${PANDOC}")

	file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/manuals.txt man_list)

	foreach(man ${man_list})
		configure_man(${man} ${CMAKE_CURRENT_SOURCE_DIR}/${man}.md)
	endforeach(man man_list)

	add_manpage_links(data_mover_dml_new.3
		data_mover_dml_delete)

	add_manpage_links(data_mover_sync_new.3
		data_mover_sync_delete)

	add_manpage_links(data_mover_threads_new.3
		data_mover_threads_delete)

	add_manpage_links(miniasync_future.7
		FUTURE FUTURE_INIT FUTURE_AS_RUNNABLE FUTURE_OUTPUT FUTURE_CHAIN_ENTRY
		FUTURE_CHAIN_ENTRY_INIT FUTURE_BUSY_POLL FUTURE_CHAIN_INIT)

	add_manpage_links(runtime_new.3
		runtime_delete)

	add_manpage_links(runtime_wait.3
		runtime_wait_multiple)

	# install manpages
	install(DIRECTORY ${MAN_DIR}/
		DESTINATION ${CMAKE_INSTALL_MANDIR}/man7
		FILES_MATCHING
		PATTERN "*.7"
		PATTERN "tmp" EXCLUDE)
	install(DIRECTORY ${MAN_DIR}/
		DESTINATION ${CMAKE_INSTALL_MANDIR}/man3
		FILES_MATCHING
		PATTERN "*.3"
		PATTERN "tmp" EXCLUDE)

	add_custom_target(doc ALL
		DEPENDS ${MANPAGE_OUTFILES}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
else()
	message(WARNING "pandoc not found - man pages (C documentation) will not be generated"
			"If you wish to build them install pandoc, "
			"otherwise disable it using CMake option -DBUILD_DOC=OFF")
endif()
