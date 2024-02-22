
set(FIND_FILES_H
	"ImageFormats/NRG/NRGParser.h"
	"ImageFormats/NRG/NRGRecordSet.h"
	"ImageFormats/NRG/NRGStructures.h"
)
list(APPEND HEADER_FILES ${FIND_FILES_H})
set(FIND_FILES_S
	"ImageFormats/NRG/NRGParser.cpp"
)
list(APPEND SOURCE_FILES ${FIND_FILES_S})
set(FIND_FILES_R
)
source_group("ImageFormats\\NRG" FILES ${FIND_FILES_H} ${FIND_FILES_S})

set(FIND_FILES_H
	"ImageFormats/MDS/MDSFormats.h"
	"ImageFormats/MDS/MDSParser.h"
	"ImageFormats/MDS/MDSRecordSet.h"
)
list(APPEND HEADER_FILES ${FIND_FILES_H})
set(FIND_FILES_S
	"ImageFormats/MDS/MDSParser.cpp"
)
list(APPEND SOURCE_FILES ${FIND_FILES_S})
set(FIND_FILES_R
)
source_group("ImageFormats\\MDS" FILES ${FIND_FILES_H} ${FIND_FILES_S})

set(FIND_FILES_H
	"ImageFormats/Filesystems/miniudf.h"
	"ImageFormats/Filesystems/UDFHelpers.h"
	"ImageFormats/Filesystems/UDFStructures.h"
)
list(APPEND HEADER_FILES ${FIND_FILES_H})
set(FIND_FILES_S
	"ImageFormats/Filesystems/miniudf.cpp"
	"ImageFormats/Filesystems/UDFHelpers.cpp"
)
list(APPEND SOURCE_FILES ${FIND_FILES_S})
set(FIND_FILES_R
)
source_group("ImageFormats\\Filesystems" FILES ${FIND_FILES_H} ${FIND_FILES_S})

set(FIND_FILES_H
	"ImageFormats/CCDParser.h"
	"ImageFormats/CUEParser.h"
	"ImageFormats/ImageFormats.h"
	"ImageFormats/ImageHelpers.h"
	"ImageFormats/IMGParser.h"
	"ImageFormats/MultiTrackImage.h"
	"ImageFormats/RawCDFormats.h"
	"ImageFormats/stdafx.h"
	"ImageFormats/UDFAnalyzer.h"
)
list(APPEND HEADER_FILES ${FIND_FILES_H})
set(FIND_FILES_S
	"ImageFormats/CCDParser.cpp"
	"ImageFormats/CUEParser.cpp"
	"ImageFormats/ImageFormats.cpp"
	"ImageFormats/IMGParser.cpp"
	"ImageFormats/RawCDFormats.cpp"
	"ImageFormats/stdafx.cpp"
	"ImageFormats/UDFAnalyzer.cpp"
)
list(APPEND SOURCE_FILES ${FIND_FILES_S})
set(FIND_FILES_R
)
source_group("ImageFormats" FILES ${FIND_FILES_H} ${FIND_FILES_S})

#[[
set(FIND_FILES_H
	"bzshlp/Win32/apicaller.h"
	"bzshlp/Win32/BCD.h"
	"bzshlp/Win32/bulkusb.h"
	"bzshlp/Win32/bulkusbcmd.h"
	"bzshlp/Win32/bzsdev.h"
	"bzshlp/Win32/bzsreg.h"
	"bzshlp/Win32/cmdexec.h"
	"bzshlp/Win32/cmndef.h"
	"bzshlp/Win32/dblbuf.h"
	"bzshlp/Win32/display.h"
	"bzshlp/Win32/filemap.h"
	"bzshlp/Win32/findproc.h"
	"bzshlp/Win32/guid.h"
	"bzshlp/Win32/HandleHolder.h"
	"bzshlp/Win32/i18n.h"
	"bzshlp/Win32/LNGManager.h"
	"bzshlp/Win32/pepatch.h"
	"bzshlp/Win32/process.h"
	"bzshlp/Win32/RegistrySerializer.h"
	"bzshlp/Win32/ResourceReader.h"
	"bzshlp/Win32/services.h"
	"bzshlp/Win32/stdafx.h"
	"bzshlp/Win32/volmgr.h"
	"bzshlp/Win32/WMI.h"
	"bzshlp/Win32/wow64.h"
)
list(APPEND HEADER_FILES ${FIND_FILES_H})
set(FIND_FILES_S
	"bzshlp/Win32/BCD.cpp"
	"bzshlp/Win32/bulkusb.cpp"
	"bzshlp/Win32/bzsdev.cpp"
	"bzshlp/Win32/bzsreg.cpp"
	"bzshlp/Win32/cmdexec.cpp"
	"bzshlp/Win32/display.cpp"
	"bzshlp/Win32/i18n.cpp"
	"bzshlp/Win32/LNGManager.cpp"
	"bzshlp/Win32/services.cpp"
	"bzshlp/Win32/volmgr.cpp"
	"bzshlp/Win32/WMI.cpp"
	"bzshlp/Win32/wow64.cpp"
)
list(APPEND SOURCE_FILES ${FIND_FILES_S})
set(FIND_FILES_R
)
source_group("bzshlp\\Win32" FILES ${FIND_FILES_H} ${FIND_FILES_S})

set(FIND_FILES_H
	"bzshlp/alg.h"
	"bzshlp/BinarySerializer.h"
	"bzshlp/bzsdisk.h"
	"bzshlp/bzsstream.h"
	"bzshlp/cmndef.h"
	"bzshlp/endian.h"
	"bzshlp/filters.h"
	"bzshlp/gcc_Debug (Posix).h"
	"bzshlp/gcc_Release (Posix).h"
	"bzshlp/HandleHolder.h"
	"bzshlp/i18n.h"
	"bzshlp/logger.h"
	"bzshlp/MBREmulator.h"
	"bzshlp/memfile.h"
	"bzshlp/ParsingLogger.h"
	"bzshlp/ratecalc.h"
	"bzshlp/serializer.h"
	"bzshlp/ser_def.h"
	"bzshlp/smvec.h"
	"bzshlp/stdafx.h"
	"bzshlp/textfile.h"
	"bzshlp/textser.h"
	"bzshlp/uidpool.h"
	"bzshlp/UserModeWorkerBackend.h"
)
list(APPEND HEADER_FILES ${FIND_FILES_H})
set(FIND_FILES_S
	"bzshlp/ratecalc.cpp"
	"bzshlp/stdafx.cpp"
	"bzshlp/UserModeWorkerBackend.cpp"
)
list(APPEND SOURCE_FILES ${FIND_FILES_S})
set(FIND_FILES_R
)
source_group("bzshlp" FILES ${FIND_FILES_H} ${FIND_FILES_S})

set(FIND_FILES_H
	"bzscore/Win32/atomic.h"
	"bzscore/Win32/cfgstore.h"
	"bzscore/Win32/datetime.h"
	"bzscore/Win32/file.h"
	"bzscore/Win32/file_defs.h"
	"bzscore/Win32/memdbg.h"
	"bzscore/Win32/path.h"
	"bzscore/Win32/registry.h"
	"bzscore/Win32/security.h"
	"bzscore/Win32/security_common.h"
	"bzscore/Win32/status_defs.h"
	"bzscore/Win32/stdafx.h"
	"bzscore/Win32/sync.h"
	"bzscore/Win32/thread.h"
	"bzscore/Win32/tls.h"
)
list(APPEND HEADER_FILES ${FIND_FILES_H})
set(FIND_FILES_S
	"bzscore/Win32/path.cpp"
	"bzscore/Win32/registry_win32.cpp"
	"bzscore/Win32/security.cpp"
	"bzscore/Win32/win32_status.cpp"
	"bzscore/Win32/win32_string.cpp"
)
list(APPEND SOURCE_FILES ${FIND_FILES_S})
set(FIND_FILES_R
)
source_group("bzscore\\Win32" FILES ${FIND_FILES_H} ${FIND_FILES_S})

set(FIND_FILES_H
	"bzscore/assert.h"
	"bzscore/atomic.h"
	"bzscore/autofile.h"
	"bzscore/autolock.h"
	"bzscore/buffer.h"
	"bzscore/bzscore.h"
	"bzscore/cfgstore.h"
	"bzscore/cmndef.h"
	"bzscore/datetime.h"
	"bzscore/file.h"
	"bzscore/fileflags.h"
	"bzscore/filehlp.h"
	"bzscore/gcc_Debug (KEXT).h"
	"bzscore/minmax.h"
	"bzscore/objcoll.h"
	"bzscore/objmgr.h"
	"bzscore/path.h"
	"bzscore/platform.h"
	"bzscore/SCSI.h"
	"bzscore/socket.h"
	"bzscore/socket_common.h"
	"bzscore/splitstr.h"
	"bzscore/status.h"
	"bzscore/stdafx.h"
	"bzscore/strbase.h"
	"bzscore/strfast.h"
	"bzscore/string.h"
	"bzscore/strop.h"
	"bzscore/sync.h"
	"bzscore/tchar_compat.h"
	"bzscore/thread.h"
)
list(APPEND HEADER_FILES ${FIND_FILES_H})
set(FIND_FILES_S
	"bzscore/security.cpp"
	"bzscore/stdafx.cpp"
)
list(APPEND SOURCE_FILES ${FIND_FILES_S})
set(FIND_FILES_R
)
source_group("bzscore" FILES ${FIND_FILES_H} ${FIND_FILES_S})
]]

set(INCLUDE_DEFINE 
	"${CMAKE_CURRENT_SOURCE_DIR}"
	#"${CMAKE_CURRENT_SOURCE_DIR}/ImageFormats"
	#"${CMAKE_CURRENT_SOURCE_DIR}/bzshlp"
	#"${CMAKE_CURRENT_SOURCE_DIR}/bzscore"
)
#[[
set(OUT_FILENAME "CDImage")

add_executable(${OUT_FILENAME} WIN32 ${SOURCE_FILES} ${RESOURCE_FILES})
MACRO_SET_OUT_PATH(${OUT_FILENAME})
target_include_directories("${OUT_FILENAME}" PRIVATE
	${INCLUDE_DEFINE}
	)
target_compile_definitions("${OUT_FILENAME}" PRIVATE
UNICODE _UNICODE
)
]]
set(OUT_FILENAME "libCDImage")

add_library(${OUT_FILENAME} STATIC ${SOURCE_FILES} ${RESOURCE_FILES})
MACRO_SET_OUT_PATH(${OUT_FILENAME})
set_property(TARGET "${OUT_FILENAME}" PROPERTY FOLDER "${LIBRARY_FOLDER}")
target_include_directories("${OUT_FILENAME}" PUBLIC
	${INCLUDE_DEFINE}
	)
target_compile_definitions("${OUT_FILENAME}" PRIVATE
UNICODE _UNICODE
)
