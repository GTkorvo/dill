/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if bfd functions use bfd_byte arguments. */
#undef BFD_BYTE

/* Define to 1 if the emulator should be built */
#undef BUILD_EMULATOR

/* Define if dill should attempt to DCG */
#undef EMULATION_ONLY

/* Define to 1 if you have the <dis-asm.h> header file. */
#cmakedefine HAVE_DIS_ASM_H

/* Define to 1 if you have the <dlfcn.h> header file. */
#undef HAVE_DLFCN_H

/* Define to 1 if you have the <inttypes.h> header file. */
#undef HAVE_INTTYPES_H

/* Define to 1 if you have the <malloc.h> header file. */
#cmakedefine HAVE_MALLOC_H

/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine HAVE_MEMORY_H

/* Define if you have the `print_insn_arm' function. */
#undef HAVE_PRINT_INSN_ARM

/* Define if you have the `print_insn_big_powerpc' function. */
#undef HAVE_PRINT_INSN_BIG_POWERPC

/* Define if you have the `print_insn_i386' function. */
#undef HAVE_PRINT_INSN_I386

/* Define if you have the `print_insn_ia64' function. */
#undef HAVE_PRINT_INSN_IA64

/* Define if you have the `print_insn_little_arm' function. */
#undef HAVE_PRINT_INSN_LITTLE_ARM

/* Define if you have the `print_insn_sparc' function. */
#undef HAVE_PRINT_INSN_SPARC

/* Define to 1 if you have the <stdint.h> header file. */
#undef HAVE_STDINT_H

/* Define to 1 if you have the <stdlib.h> header file. */
#undef HAVE_STDLIB_H

/* Define to 1 if you have the <strings.h> header file. */
#undef HAVE_STRINGS_H

/* Define to 1 if you have the <string.h> header file. */
#undef HAVE_STRING_H

/* Define to 1 if you have the <sys/stat.h> header file. */
#undef HAVE_SYS_STAT_H

/* Define to 1 if you have the <sys/types.h> header file. */
#undef HAVE_SYS_TYPES_H

/* Define to 1 if you have the <unistd.h> header file. */
#undef HAVE_UNISTD_H

/* Define if the host processor is ARM */
#cmakedefine HOST_ARM

/* Define if the host processor is an ia64 */
#cmakedefine HOST_IA64

/* Define if the host processor is a powerpc */
#cmakedefine HOST_POWERPC

/* Define if the host processor is a powerpc64 */
#cmakedefine HOST_POWERPC64

/* Define if the host processor is a sparc */
#cmakedefine HOST_SPARC

/* Define if the host processor is a sparcv9 */
#cmakedefine HOST_SPARCV9

/* Define if the host processor is an x86 */
#cmakedefine HOST_X86

/* Define if the host processor is an x86_64 */
#cmakedefine HOST_X86_64

/* Define if INIT_DISASSEMBLE_INFO takes three arguments instead of two */
#cmakedefine INIT_DISASSEMBLE_INFO_THREE_ARG

/* Define if integrating with kernel plugins */
#undef KPLUGINS_INTEGRATION

/* Define if compiling for linux kernel */
#undef LINUX_KERNEL_MODULE

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#undef LT_OBJDIR

/* Define if you want more than just native target */
#undef MULTI_TARGET

/* Define for the host architecture type */
#cmakedefine NATIVE_ARCH "@NATIVE_ARCH@"

/* Define if there is no disassembler */
#cmakedefine NO_DISASSEMBLER

/* Define if we should not use inlined procedures from BFD */
#undef NO_INLINED_BFD_PROCS

/* The number of bytes in type long */
#undef SIZEOF_LONG

/* Define to 1 if you have the ANSI C header files. */
#cmakedefine STDC_HEADERS

/* Define if the membar instruction should be used to sync Icache and Dcache
   */
#undef USE_MEMBAR

/* Define this is mmap should be used instead of malloc() for code memory */
#cmakedefine USE_MMAP_CODE_SEG

/* Define if byteorder is bigendian */
#cmakedefine WORDS_BIGENDIAN

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
#undef inline
#endif
