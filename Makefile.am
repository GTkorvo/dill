## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = no-dependencies foreign

SUBDIRS = . tests vtests

BUILT_SOURCES = dill.h
if BUILD_LIBRARY
lib_LTLIBRARIES = libdill.la
include_HEADERS = dill.h
endif BUILD_LIBRARY

all-local:
check-local:
install-exec-local:

noinst_HEADERS = config.h dill_internal.h

if MULTI_TARGET
libdill_la_SOURCES = dill_sparc.c dill_util.c dill_cplus.c dill_pkg.c sparc.c dill_x86.c x86.c x86_rt.c \
		dill_arm5.c arm5.c dill_virtual.c dill_x86_64.c x86_64.c \
		dill_ia64.c ia64.c virtual.c dill.c vm.c version.c
else !MULTI_TARGET
if NATIVE_CG
libdill_la_SOURCES = dill_util.c dill_pkg.c dill_cplus.c dill_virtual.c virtual.c dill.c vm.c dill_${ARCH_FILE}.c ${ARCH_FILE}.c ${ARCH_FILE}_rt.c version.c
else !NATIVE_CG
libdill_la_SOURCES = dill_util.c dill_pkg.c dill_cplus.c dill_virtual.c virtual.c dill.c vm.c version.c
endif !NATIVE_CG
endif !MULTI_TARGET

libdill_la_LIBADD =   @DIS_LIBS@ 
libdill_la_DEPENDENCIES = @DUMMY_LIBOPCODES_TARGET@
libdill_la_LDFLAGS =

CLEANFILES = dill_sparc.c dill.h dill_x86.c dill_x86_64.c dill_arm5.c dill_virtual.c dill.c

vm.lo: vm_include.c

dill_sparc.c: sparc.ops
	$(PERL) $(srcdir)/sparc.ops

dill_virtual.c: virtual.ops
	$(PERL) $(srcdir)/virtual.ops

dill_x86.c: x86.ops
	$(PERL) $(srcdir)/x86.ops

dill_x86_64.c: x86_64.ops
	$(PERL) $(srcdir)/x86_64.ops

dill_ia64.c: ia64.ops
	$(PERL) $(srcdir)/ia64.ops

dill_arm5.c: arm5.ops
	$(PERL) $(srcdir)/arm5.ops

dill.h: base.ops
	$(PERL) $(srcdir)/base.ops

$(libdill_la_OBJECTS) : dill.h

libopcodes.la:
	./make_dummy_la @LIBOPCODES_DIR@ opcodes
