cmake_minimum_required(VERSION 2.8)
project(dill)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(CheckFunctionExists)
include(CheckIncludeFiles)
include(Compile)

set (SRC_LIST dill_util.c dill_pkg.c dill_cplus.c dill_virtual.c virtual.c dill.c vm.c version.c)

IF( NOT CMAKE_BUILD_TYPE )
    SET( CMAKE_BUILD_TYPE "RelWithDebInfo" )
ENDIF()

set (NATIVE_CG true)
if (("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "i.86") OR
	("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "x86_64"))
    if (CMAKE_SIZEOF_VOID_P MATCHES "8")
      	set (NATIVE_ARCH x86_64)
	set (HOST_X86_64 1)
	set (TEST_PERL_FLAGS "-max_arg=4")
	set (ARCH_FILE x86_64)
    else (CMAKE_SIZEOF_VOID_P MATCHES "8")
	set (NATIVE_ARCH x86)
	set (HOST_X86 1)
	set (TEST_PERL_FLAGS "-max_arg=2")
	set (ARCH_FILE x86)
    endif (CMAKE_SIZEOF_VOID_P MATCHES "8")
else ()
    set (NATIVE_CG false)
endif()

IF ((${CMAKE_SYSTEM_NAME} MATCHES "Darwin") OR (${CMAKE_SYSTEM_NAME} MATCHES "Linux"))
    set (USE_MMAP_CODE_SEG 1)
ENDIF()

option(ENABLE_DISASSEMBLY "enable binutils-based disassembly (default is OFF)"
         OFF)

set (ARCHITECTURES sparc powerpc virtual x86 x86_64 ia64 arm5)
foreach(_arch ${ARCHITECTURES})
  add_custom_command(
    OUTPUT "dill_${_arch}.c"
    COMMAND perl ${CMAKE_CURRENT_SOURCE_DIR}/${_arch}.ops
    DEPENDS ${_arch}.ops
  ) 
  list(APPEND arch_files "dill_${_arch}.c" "${_arch}.c"})
endforeach()

add_custom_command(
  OUTPUT "dill.h" "dill.c"
  COMMAND perl ${CMAKE_CURRENT_SOURCE_DIR}/base.ops
  DEPENDS base.ops
) 

OPTION(MULTI_TARGET  "Build all available target architectures" OFF)

if (MULTI_TARGET)
    set (SRC_LIST ${SRC_LIST} ${arch_files})
elseif (DEFINED NATIVE_ARCH) 
   set (SRC_LIST ${SRC_LIST} dill_${ARCH_FILE}.c ${ARCH_FILE}.c ${ARCH_FILE}_rt.c)
endif (MULTI_TARGET)

if (MULTI_TARGET OR NATIVE_CG OR EMULATION_POSSIBLE)
    if (EMULATION_POSSIBLE)
	set (EMU_LIB avcall)
    endif(EMULATION_POSSIBLE)
    if ((NOT MULTI_TARGET) AND (NOT NATIVE_CG))
        message ( STATUS Configure has detected no native dynamic code generation support 
	for this architecture (\"$NATIVE_ARCH\").  However, the avcall
	library was found, so library configure for emulation will be built.)
	set (EMULATION_ONLY true)
    endif()
else ()
    message ( FATAL_ERROR "Configure has detected no native dynamic code generation support 
	for this architecture ("$NATIVE_ARCH"), --enable-multitarget 
	was not specified, and no emulation is possible (avcall library 
	not found)
		       NO LIBRARY WILL BE BUILT")
endif()

add_library(dill SHARED ${SRC_LIST} dill.h)
add_library(dill-static STATIC ${SRC_LIST} dill.h)
# The library target "dill" already has a default OUTPUT_NAME of "dill", so we don't need to change it.
# The library target "dill-static" has a default OUTPUT_NAME of "dill-static", so change it.
SET_TARGET_PROPERTIES(dill-static PROPERTIES OUTPUT_NAME "dill")
# Now the library target "foo-static" will be named "foo.lib" with MS tools.
# This conflicts with the "foo.lib" import library corresponding to "foo.dll",
# so we add a "lib" prefix (which is default on other platforms anyway):
SET_TARGET_PROPERTIES(dill-static PROPERTIES PREFIX "lib")

CHECK_INCLUDE_FILES(unistd.h HAVE_UNISTD_H)

CHECK_INCLUDE_FILES(stdarg.h STDC_HEADERS)
CHECK_INCLUDE_FILES(malloc.h HAVE_MALLOC_H)
CHECK_INCLUDE_FILES(memory.h HAVE_MEMORY_H)

set (NO_DISASSEMBLER TRUE)
if (ENABLE_DISASSEMBLY)
    FIND_PACKAGE (BinUtils)
    if (NOT BINUTILS_LIBRARIES)
	message(FATAL_ERROR "libopcodes not found, You need to install binutils for disassembler support.")
    else()
	INCLUDE_DIRECTORIES(${BINUTILS_INCLUDE_DIR})
	LINK_DIRECTORIES(${BINUTILS_LIB_DIR})
	TARGET_LINK_LIBRARIES(dill ${BINUTILS_LIBRARIES})
	TARGET_LINK_LIBRARIES(dill-static opcodes)
	set (NO_DISASSEMBLER FALSE)
    endif()
    if(NOT DEFINED INIT_DISASSEMBLE_INFO_THREE_ARG)
	#if not in Cache
	set (INIT_DISASSEMBLE_INFO_THREE_ARG OFF CACHE BOOL "Does the INIT_DISASSEMBLE_INFO macro require three arguments")
    	FILE (WRITE check_init_disassemble_args.c "#include \"dis-asm.h\"
extern int main() {struct disassemble_info info;INIT_DISASSEMBLE_INFO(info, stdout, fprintf);}" )
    	COMPILE (INIT_DISASSEMBLE_INFO_THREE_ARG ${CMAKE_CURRENT_SOURCE_DIR}/check_init_disassemble_args.c)
    	FILE (REMOVE check_init_disassemble_args.c)
    endif()
endif()


CHECK_INCLUDE_FILES(dis-asm.h HAVE_DIS_ASM_H)
INSTALL(FILES ${CURRENT_BINARY_DIR}/dill.h DESTINATION include)
INSTALL(TARGETS dill dill-static
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

CONFIGURE_FILE( ${CMAKE_SOURCE_DIR}/config.h.cmake ${CMAKE_SOURCE_DIR}/config.h )

# display status message for important variables
MESSAGE( STATUS )
MESSAGE( STATUS "-------------------------------------------------------------------------------" )
MESSAGE( STATUS "CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}" )
MESSAGE( STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}" )
MESSAGE( STATUS "Change a value with: cmake -D<Variable>=<Value>" )
MESSAGE( STATUS "-------------------------------------------------------------------------------" )

ENABLE_TESTING()
ADD_SUBDIRECTORY( tests )
ADD_SUBDIRECTORY( vtests )

