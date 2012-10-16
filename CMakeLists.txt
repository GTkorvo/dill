cmake_minimum_required(VERSION 2.8.3)
cmake_policy(VERSION 2.8.3)
project(dill)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(CheckCSourceCompiles)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckIncludeFiles)
include(CheckTypeSize)
include(CreateLibtoolFile)
include(Compile)
include(FindCERCSProject)

set (SRC_LIST dill_util.c dill_pkg.c dill_cplus.c dill_virtual.c virtual.c dill.c vm.c version.c)

IF( NOT CMAKE_BUILD_TYPE )
    SET( CMAKE_BUILD_TYPE "RelWithDebInfo" )
ENDIF()

CHECK_TYPE_SIZE("void*" CMAKE_SIZEOF_VOID_P)
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
	set (TEST_PERL_FLAGS -no_float -max_arg=2)
	set (ARCH_FILE x86)
    endif (CMAKE_SIZEOF_VOID_P MATCHES "8")
elseif ("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "arm.*")
    if ("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "arm.5.*")
	set (NATIVE_ARCH arm5)
	set (HOST_ARM5 1)
	set (TEST_PERL_FLAGS -max_arg=4)
	set (ARCH_FILE arm5)
    elseif ("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "arm.6.")
	set (NATIVE_ARCH arm6)
	set (HOST_ARM6 1)
	set (TEST_PERL_FLAGS -max_arg=4)
	set (ARCH_FILE arm6)
    else ()
      message(status "case 3  ${CMAKE_SYSTEM_PROCESSOR}")
	set (NATIVE_ARCH arm6)
	set (HOST_ARM6 1)
	set (TEST_PERL_FLAGS -max_arg=4)
	set (ARCH_FILE arm6)
    endif()
else()
message (STATUS "NO CG for Processor ${CMAKE_SYSTEM_PROCESSOR}")
    set (NATIVE_CG false)
endif()

IF ((${CMAKE_SYSTEM_NAME} MATCHES "Darwin") OR (${CMAKE_SYSTEM_NAME} MATCHES "Linux"))
    set (USE_MMAP_CODE_SEG 1)
ENDIF()

option(BUILD_STATIC "enable building the static library" ON)
option(BUILD_DYNAMIC "enable building the dynamic library" ON)
option(ENABLE_DISASSEMBLY "enable binutils-based disassembly (default is OFF)"
         OFF)

set (ARCHITECTURES sparc powerpc virtual x86 x86_64 ia64 arm5 arm6)
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
	for this architecture ("$NATIVE_ARCH"), -DMULTI_TARGET=ON 
	was not specified, and no emulation is possible (avcall library 
	not found)
		       NO LIBRARY WILL BE BUILT")
endif()

set (INSTALL_TARGETS)
if (BUILD_DYNAMIC)
  add_library(dill SHARED ${SRC_LIST} dill.h)
  SET_TARGET_PROPERTIES(dill PROPERTIES LT_SHOULDNOTLINK "no")
  if (BUILD_DYNAMIC)
    set_target_properties(dill PROPERTIES STATIC_LIB "libdill.a")
  endif (BUILD_DYNAMIC)
  LIST(APPEND INSTALL_TARGETS dill)
endif (BUILD_DYNAMIC)
if (BUILD_STATIC)
  add_library(dill-static STATIC ${SRC_LIST} dill.h)
  # The library target "dill" already has a default OUTPUT_NAME of "dill", so we don't need to change it.
  # The library target "dill-static" has a default OUTPUT_NAME of "dill-static", so change it.
  SET_TARGET_PROPERTIES(dill-static PROPERTIES OUTPUT_NAME "dill")
  # Now the library target "foo-static" will be named "foo.lib" with MS tools.
  # This conflicts with the "foo.lib" import library corresponding to "foo.dll",
  # so we add a "lib" prefix (which is default on other platforms anyway):
  SET_TARGET_PROPERTIES(dill-static PROPERTIES PREFIX "lib")
  LIST(APPEND INSTALL_TARGETS dill-static)
endif (BUILD_STATIC)

CHECK_INCLUDE_FILES(unistd.h HAVE_UNISTD_H)

CHECK_INCLUDE_FILES(stdarg.h STDC_HEADERS)
CHECK_INCLUDE_FILES(malloc.h HAVE_MALLOC_H)
CHECK_INCLUDE_FILES(memory.h HAVE_MEMORY_H)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set (NO_DISASSEMBLER TRUE)
if (ENABLE_DISASSEMBLY)
#    FIND_PACKAGE (BinUtils)
    FIND_CERCS_PROJECT(binutils LIBRARY opcodes INCLUDES dis-asm.h REQUIRED)
    if (NOT BINUTILS_LIBRARIES)
	message(FATAL_ERROR "libopcodes not found, You need to install binutils for disassembler support.")
    else()
	INCLUDE_DIRECTORIES(${BINUTILS_INCLUDE_DIR})
	LINK_DIRECTORIES(${BINUTILS_LIB_DIR})
	if (BUILD_DYNAMIC)
	  TARGET_LINK_LIBRARIES(dill ${BINUTILS_LIBRARIES})
	endif (BUILD_DYNAMIC)
	if (BUILD_STATIC)
	  TARGET_LINK_LIBRARIES(dill-static opcodes)
	endif (BUILD_STATIC)
	set (NO_DISASSEMBLER FALSE)
    endif()
    if(NOT DEFINED INIT_DISASSEMBLE_INFO_THREE_ARG)
	#if not in Cache
    	FILE (WRITE check_init_disassemble_args.c "#include \"dis-asm.h\"
extern int main() {struct disassemble_info info;INIT_DISASSEMBLE_INFO(info, stdout, fprintf);}" )
    	TRY_COMPILE (INIT_DISASSEMBLE_INFO_THREE_ARGUMENTS ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/check_init_disassemble_args.c CMAKE_FLAGS "-DCMAKE_C_LINK_EXECUTABLE=echo" COMPILE_DEFINITIONS -I${BINUTILS_INCLUDE_DIR} OUTPUT_VARIABLE LOG2)
    	FILE (REMOVE check_init_disassemble_args.c)
	message (STATUS "Checking to see that the INIT_DISASSEMBLE_INFO macro requires three arguments :${INIT_DISASSEMBLE_INFO_THREE_ARGUMENTS}")
	set (INIT_DISASSEMBLE_INFO_THREE_ARG ${INIT_DISASSEMBLE_INFO_THREE_ARGUMENTS} CACHE BOOL "Does the INIT_DISASSEMBLE_INFO macro require three arguments")
    endif()
    CHECK_LIBRARY_EXISTS(opcodes print_insn_sparc "${BINUTILS_LIB_DIR}" HAVE_PRINT_INSN_SPARC)
    CHECK_LIBRARY_EXISTS(opcodes print_insn_big_powerpc "${BINUTILS_LIB_DIR}" HAVE_PRINT_INSN_BIG_POWERPC)
    CHECK_LIBRARY_EXISTS(opcodes print_insn_ia64 "${BINUTILS_LIB_DIR}" HAVE_PRINT_INSN_IA64)
    CHECK_LIBRARY_EXISTS(opcodes print_insn_i386 "${BINUTILS_LIB_DIR}" HAVE_PRINT_INSN_I386)
    CHECK_LIBRARY_EXISTS(opcodes print_insn_arm "${BINUTILS_LIB_DIR}" HAVE_PRINT_INSN_ARM)
    CHECK_LIBRARY_EXISTS(opcodes print_insn_little_arm "${BINUTILS_LIB_DIR}" HAVE_PRINT_INSN_LITTLE_ARM)

endif()


CREATE_LIBTOOL_FILE(dill /lib)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/dill.h DESTINATION include)
install(TARGETS ${INSTALL_TARGETS}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

if (${CMAKE_C_COMPILER_ID} MATCHES "Intel") 
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -shared-intel")
endif()

CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

# display status message for important variables
MESSAGE( STATUS )
MESSAGE( STATUS "-------------------------------------------------------------------------------" )
MESSAGE( STATUS "CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}" )
MESSAGE( STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}" )
MESSAGE( STATUS "BUILD_STATIC = ${BUILD_STATIC}" )
MESSAGE( STATUS "BUILD_DYNAMIC = ${BUILD_DYNAMIC}" )
MESSAGE( STATUS "ENABLE_DISASSEMBLY = ${ENABLE_DISASSEMBLY}" )
MESSAGE( STATUS "MULTI_TARGET = ${MULTI_TARGET}" )
MESSAGE( STATUS "Change a value with: cmake -D<Variable>=<Value>" )
MESSAGE( STATUS "-------------------------------------------------------------------------------" )

INCLUDE(CTest)
ENABLE_TESTING()
IF(BUILD_TESTING)
  SET(BUILDNAME "${BUILDNAME}" CACHE STRING "Name of build on the dashboard")
  MARK_AS_ADVANCED(BUILDNAME)
ENDIF(BUILD_TESTING)

ADD_SUBDIRECTORY( tests )
ADD_SUBDIRECTORY( vtests )

