#
#  BinUtils_FOUND - system has the BinUtils library
#  BinUtils_INCLUDE_DIR - the BinUtils include directory
#  BinUtils_LIBRARIES - The libraries needed to use BinUtils

find_library(BinUtils_opcodes_LIBRARY opcodes)
find_library(BinUtils_bfd_LIBRARY bfd)
find_path(BinUtils_INCLUDE_DIR dis-asm.h)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BinUtils DEFAULT_MSG
  BinUtils_opcodes_LIBRARY BinUtils_bfd_LIBRARY BinUtils_INCLUDE_DIR
)
if(BinUtils_FOUND)
  set(BinUtils_INCLUDE_DIRS ${BinUtils_INCLUDE_DIR})
  set(BinUtils_LIBRARIES ${BinUtils_opcodes_LIBRARY} ${BinUtils_bfd_LIBRARY})
endif()
