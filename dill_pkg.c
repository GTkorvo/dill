#include "config.h"
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include "dill.h"
#include "dill_internal.h"

static void
unpack_package(char *package, call_t *t, char **code_p)
{
    int count;
    char *p = package + sizeof(struct dill_pkg_1);
    struct dill_pkg_1 *pkg = (struct dill_pkg_1 *) package;
    if (pkg->magic != 0xbeef) printf("Not valid package\n");
    if (pkg->pkg_version != 1) printf("Not valid package version\n");
    t->call_alloc = t->call_count = pkg->symbol_count;
    t->call_locs = malloc(sizeof(t->call_locs[0]) * pkg->symbol_count);
    for (count = 0; count<pkg->symbol_count; count++) {
	int call_len;
	t->call_locs[count].loc = *((int*)p);
	t->call_locs[count].xfer_name = (p + sizeof(int));
	call_len = sizeof(int) + strlen(t->call_locs[count].xfer_name) + 1;
	call_len = (call_len + 3) & -4;  /* round up to mod 4 */
	p += call_len;
    }
    *code_p = p;
}

extern char * sparc_package_stitch(char *code, call_t *t, dill_pkg pkg);
extern char * sparcv9_package_stitch(char *code, call_t *t, dill_pkg pkg);
extern char * x86_package_stitch(char *code, call_t *t, dill_pkg pkg);
extern char * x86_64_package_stitch(char *code, call_t *t, dill_pkg pkg);
extern char * arm_package_stitch(char *code, call_t *t, dill_pkg pkg);
extern char * ia64_package_stitch(char *code, call_t *t, dill_pkg pkg);

extern void
dill_lookup_xfer_addrs(call_t *t, xfer_entry *x)
{
    int i;
    for (i = 0; i < t->call_count; i++) {
	xfer_entry *e = x;
	while(e->xfer_name != NULL) {
	    if (strcmp(e->xfer_name, t->call_locs[i].xfer_name) == 0) {
		t->call_locs[i].xfer_addr = e->xfer_addr;
	    }
	    e++;
	}
    }
}

extern void *
dill_package_stitch(char *pkg)
{
    char *code;
    call_t t;
    unpack_package(pkg, &t, &code);
/*    lookup_xfer_addrs(&t, user_addrs);*/
#if defined(HOST_X86)
    char *p = x86_package_stitch(code, &t, (dill_pkg) pkg);
#endif
#if defined(HOST_SPARC)
    char *p = sparc_package_stitch(code, &t, (dill_pkg) pkg);
#endif
#if defined(HOST_SPARCV9)
    char *p = sparcv9_package_stitch(code, &t, (dill_pkg) pkg);
#endif
#if defined(HOST_X86_64)
    char *p = x86_64_package_stitch(code, &t, (dill_pkg) pkg);
#endif
#if defined(HOST_IA64)
    char *p = ia64_package_stitch(code, &t, (dill_pkg) pkg);
#endif
#if defined(HOST_ARM5)
    char *p = arm5_package_stitch(code, &t, (dill_pkg) pkg);
#endif
    free(t.call_locs);
    return p;
}

