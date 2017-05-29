cmake_minimum_required(VERSION 2.8.3)
cmake_policy(VERSION 2.8.3)
project(dill C CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(CheckCSourceCompiles)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckIncludeFiles)
include(CheckTypeSize)
include(CreateLibtoolFile)
include(Compile)
include(FindCERCSProject)
include(AddLibtoolLibrary)
include(TestBigEndian)
include(CERCS_package)


# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


# the RPATH to be used when installing, but only if it's not a system directory
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
   SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
ENDIF("${isSystemDir}" STREQUAL "-1")

set (CMAKE_MACOSX_RPATH 1)

file (STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/version.c" VERSION_TEXT REGEX "^static.*")
STRING(REGEX REPLACE ".*Version ([0-9]+.[0-9]+.[0-9]+).*" "\\1" VERSION_TEXT "${VERSION_TEXT}")
STRING(REGEX REPLACE "([0-9]+)[0-9.]*$" "\\1" MAJOR_VERSION_STRING ${VERSION_TEXT})

set (SRC_LIST dill_util.c dill_pkg.c dill_cplus.c dill_virtual.c virtual.c dill.c vm.c version.c)

IF( NOT CMAKE_BUILD_TYPE )
    SET( CMAKE_BUILD_TYPE "RelWithDebInfo" )
ENDIF()

CHECK_TYPE_SIZE("void*" CMAKE_SIZEOF_VOID_P)
CHECK_TYPE_SIZE("long" SIZEOF_LONG)
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
    elseif ("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "arm.7.")
	set (NATIVE_ARCH arm6)
	set (HOST_ARM7 1)
	set (TEST_PERL_FLAGS -max_arg=4)
	set (ARCH_FILE arm6)
    else ()
	set (NATIVE_ARCH arm6)
	set (HOST_ARM6 1)
	set (TEST_PERL_FLAGS -max_arg=4)
	set (ARCH_FILE arm6)
    endif()
elseif ("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "aarch64")
    set (NATIVE_ARCH arm8)
    set (HOST_ARM8 1)
    set (ARCH_FILE arm8)
else()
    message (STATUS "NO CG for Processor \"${CMAKE_SYSTEM_PROCESSOR}\"")
    set (NATIVE_CG false)
    set (NATIVE_ARCH UNSUPPORTED)
endif()

IF ((${CMAKE_SYSTEM_NAME} MATCHES "Darwin") OR (${CMAKE_SYSTEM_NAME} MATCHES "Linux"))
    set (USE_MMAP_CODE_SEG 1)
ENDIF()

TEST_BIG_ENDIAN(WORDS_BIGENDIAN)
set (HARDFP_AVAILABLE 0)
# Determine float ABI of ARM Linux
if (CMAKE_SYSTEM_PROCESSOR MATCHES "^arm" AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_program(READELF readelf)
    if (READELF MATCHES "NOTFOUND")
        message(WARNING "readelf not found; float ABI detection disabled")
    else (READELF MATCHES "NOTFOUND")
        find_library(MATH_LIBRARY m /usr/lib)
        execute_process(
            COMMAND ${READELF} -A ${MATH_LIBRARY}
            OUTPUT_VARIABLE ELF_ARCH
            ERROR_QUIET)
        if (ELF_ARCH MATCHES "Tag_ABI_VFP_args: VFP registers")
	    set(HARDFP_AVAILABLE 1)
        endif ()
    endif (READELF MATCHES "NOTFOUND")
endif (CMAKE_SYSTEM_PROCESSOR MATCHES "^arm" AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
set(ARM_HARD_FLOAT ${HARDFP_AVAILABLE})

find_package (LibFFI PATHS ${CMAKE_INSTALL_PREFIX})
IF (LIBFFI_FOUND) 
  set (EMULATION_POSSIBLE true)
ENDIF (LIBFFI_FOUND) 
IF(NOT DEFINED BUILD_SHARED_STATIC)
  SET( BUILD_SHARED_STATIC "BOTH")
ENDIF(NOT DEFINED BUILD_SHARED_STATIC)
option(ENABLE_DISASSEMBLY "enable binutils-based disassembly (default is OFF)"
         OFF)

set (ARCHITECTURES sparc powerpc virtual x86 x86_64 ia64 arm5 arm6 arm7 arm8)
foreach(_arch ${ARCHITECTURES})
  add_custom_command(
    OUTPUT "dill_${_arch}.c"
    COMMAND perl ${CMAKE_CURRENT_SOURCE_DIR}/${_arch}.ops
    DEPENDS ${_arch}.ops
  ) 
  list(APPEND arch_files "dill_${_arch}.c" "${_arch}.c" "${_arch}_rt.c")
endforeach()
list(REMOVE_ITEM arch_files virtual_rt.c)

add_custom_command(
  OUTPUT "dill.h" "dill.c"
  COMMAND perl ${CMAKE_CURRENT_SOURCE_DIR}/base.ops
  DEPENDS base.ops
) 

OPTION(MULTI_TARGET  "Build all available target architectures" OFF)

if (MULTI_TARGET)
  message (STATUS "arch files is ${arch_files}")
    set (SRC_LIST ${SRC_LIST} ${arch_files})
elseif (NOT "${NATIVE_ARCH}" STREQUAL "UNSUPPORTED") 
   set (SRC_LIST ${SRC_LIST} dill_${ARCH_FILE}.c ${ARCH_FILE}.c ${ARCH_FILE}_rt.c)
endif (MULTI_TARGET)

if (MULTI_TARGET OR NATIVE_CG OR EMULATION_POSSIBLE)
    if (EMULATION_POSSIBLE)
	set (EMU_LIB ${LIBFFI_LIBRARY})
	INCLUDE_DIRECTORIES(${LIBFFI_INCLUDE_DIR})
    endif(EMULATION_POSSIBLE)
    if ((NOT MULTI_TARGET) AND (NOT NATIVE_CG))
        message ( STATUS "Configure has detected no native dynamic code generation support 
	for this architecture (\"$NATIVE_ARCH\").  However, the libffi
	library was found, so library configure for emulation will be built.")
	set (EMULATION_ONLY true)
	set (BUILD_EMULATOR true)
	LIST(APPEND TARGET_DEP_LIBS ${EMU_LIB})
    endif()
else ()
    message ( FATAL_ERROR "Configure has detected no native dynamic code generation support 
	for this architecture (\"$NATIVE_ARCH\"), -DMULTI_TARGET=ON 
	was not specified, and no emulation is possible (avcall library 
	not found)
		       NO LIBRARY WILL BE BUILT")
endif()

CHECK_INCLUDE_FILES(unistd.h HAVE_UNISTD_H)

CHECK_INCLUDE_FILES(stdarg.h STDC_HEADERS)
CHECK_INCLUDE_FILES(malloc.h HAVE_MALLOC_H)
CHECK_INCLUDE_FILES(memory.h HAVE_MEMORY_H)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set (NO_DISASSEMBLER TRUE)
if (ENABLE_DISASSEMBLY)
    FIND_CERCS_PROJECT(binutils LIBRARY opcodes INCLUDES dis-asm.h REQUIRED)
    if (NOT BINUTILS_LIBRARIES)
	message(FATAL_ERROR "libopcodes not found, You need to install binutils for disassembler support.")
    else()
	INCLUDE_DIRECTORIES(${BINUTILS_INCLUDE_DIR})
	LINK_DIRECTORIES(${BINUTILS_LIB_DIR})
	LIST(APPEND TARGET_DEP_LIBS ${BINUTILS_LIBRARIES} opcodes)
	set(DIS_LIBS ${TARGET_DEP_LIBS})
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

add_libtool_library(NAME dill SRC_LIST ${SRC_LIST} dill.h DEP_LIBS ${TARGET_DEP_LIBS} LINK_LIBS ${TARGET_DEP_LIBS})

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
CONFIGURE_FILE(CTestCustom.ctest.in CTestCustom.ctest @ONLY)

# display status message for important variables
MESSAGE( STATUS )
MESSAGE( STATUS "-------------------------------------------------------------------------------" )
MESSAGE( STATUS "CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}" )
MESSAGE( STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}" )
MESSAGE( STATUS "BUILD_SHARED_STATIC = ${BUILD_SHARED_STATIC}" )
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

