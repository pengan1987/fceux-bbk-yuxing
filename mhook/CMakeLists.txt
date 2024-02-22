#set(PROJECT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(SOURCE_FILES "")

set(FIND_FILES
	"mhook-lib/mhook.cpp"
	"mhook-lib/mhook.h"
)
list(APPEND SOURCE_FILES ${FIND_FILES})
source_group("mhook-lib" FILES ${FIND_FILES})

set(FIND_FILES
	"disasm-lib/cpu.c"
	"disasm-lib/cpu.h"
	"disasm-lib/disasm.c"
	"disasm-lib/disasm.h"
	"disasm-lib/disasm_x86.c"
	"disasm-lib/disasm_x86.h"
	"disasm-lib/disasm_x86_tables.h"
	"disasm-lib/misc.c"
	"disasm-lib/misc.h"
)
list(APPEND SOURCE_FILES ${FIND_FILES})
source_group("disasm-lib" FILES ${FIND_FILES})

set(OUT_FILENAME "libmhook")
add_library("${OUT_FILENAME}" STATIC ${SOURCE_FILES})
target_compile_definitions("${OUT_FILENAME}" PUBLIC
    $<$<CONFIG:RRRDebug>:DEBUG_BUILD SET_DEBUG>
    $<$<CONFIG:Release>:RELEASE_BUILD SET_RELEASE>
    #UNICODE _UNICODE
)
target_include_directories("${OUT_FILENAME}" PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/mhook-lib"
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	"${CMAKE_CURRENT_SOURCE_DIR}/disasm-lib"
)
set_property(TARGET "${OUT_FILENAME}" PROPERTY FOLDER "${LIBRARY_FOLDER}")
MACRO_SET_OUT_PATH(${OUT_FILENAME})
target_link_libraries("${OUT_FILENAME}" PUBLIC)

#[[ 
set(SOURCE_FILES "")
set(FIND_FILES
	"mhook-test.cpp"
	"stdafx.cpp"
	"stdafx.h"
)
list(APPEND SOURCE_FILES ${FIND_FILES})

set(OUT_FILENAME "mhook-test")
add_executable("${OUT_FILENAME}" ${SOURCE_FILES})
set_property(TARGET "${OUT_FILENAME}" PROPERTY FOLDER "${LIBRARY_FOLDER}/mhook")
MACRO_SET_OUT_PATH(${OUT_FILENAME})

#set_property(TARGET "${OUT_FILENAME}" PROPERTY LINK_FLAGS /ENTRY:"wWinMainCRTStartup")

target_link_libraries("${OUT_FILENAME}" libmhook
	)
	]]