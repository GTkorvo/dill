#include "config.h"
#include <string.h>
#include "dill.h"
#include "dill_internal.h"
#include "arm6.h"
#include <string.h>

extern int arm6_hidden_modi(int a, int b)
{ return a % b; }
extern long arm6_hidden_mod(long a, long b)
{ return a % b; }
extern unsigned long arm6_hidden_umod(unsigned long a, unsigned long b)
{ return a % b; }
extern unsigned int arm6_hidden_umodi(unsigned int a, unsigned int b)
{ return a % b; }
extern double arm6_hidden_ultod(unsigned long a)
{ return (double) a; }
extern float arm6_hidden_ultof(unsigned long a)
{ return (float) a; }
extern unsigned long arm6_hidden_dtoul(double a)
{ return (unsigned long) a; }
extern unsigned int arm6_hidden_dtou(double a)
{ return (unsigned int) a; }
extern unsigned long arm6_hidden_ftoul(float a)
{ return (unsigned long) a; }
extern unsigned int arm6_hidden_ftou(float a)
{ return (unsigned int) a; }
extern unsigned long arm6_hidden_udiv(unsigned long a, unsigned long b)
{ return a / b; }
extern long arm6_hidden_div(long a, long b)
{ return a / b; }

static xfer_entry arm6_xfer_recs[] = {
    {"arm6_hidden_modi", arm6_hidden_modi},
    {"arm6_hidden_mod", arm6_hidden_mod},
    {"arm6_hidden_umod", arm6_hidden_umod},
    {"arm6_hidden_umodi", arm6_hidden_umodi},
    {"arm6_hidden_ultod", arm6_hidden_ultod},
    {"arm6_hidden_ultof", arm6_hidden_ultof},
    {"arm6_hidden_dtoul", arm6_hidden_dtoul},
    {"arm6_hidden_dtou", arm6_hidden_dtou},
    {"arm6_hidden_ftoul", arm6_hidden_ftoul},
    {"arm6_hidden_ftou", arm6_hidden_ftou},
    {"arm6_hidden_udiv", arm6_hidden_udiv},
    {"arm6_hidden_div", arm6_hidden_div},
    {(char*)0, (void*)0}};

extern void
arm6_rt_call_link(char *code, call_t *t)
{
    int i;

    for(i=0; i< t->call_count; i++) {
	int *call_addr = (int*) ((unsigned long)code + 
				 t->call_locs[i].loc);
	if (t->call_locs[i].mach_info == NULL) {
	    /* no PLT */
	    int call_offset = (unsigned long)t->call_locs[i].xfer_addr -
		(unsigned long)call_addr;
	
	    /* compensate for arm PC lookahead */
	    call_offset = call_offset - 8;
	    /* div addr diff by 4 for arm offset value */
	    call_offset = call_offset >> 2;
	    *call_addr &= 0xff000000;
	    *call_addr |= (call_offset & 0xffffff);
	} else {
	    /* call through PLT */
	    unsigned long PLT_addr = (unsigned long)code + 
				      (unsigned long)t->call_locs[i].mach_info;
	    int call_offset = PLT_addr - (unsigned long)call_addr;
	    
	    /* compensate for arm PC lookahead */
	    call_offset = call_offset - 8;
	    call_offset = call_offset >> 2;
	    *call_addr &= 0xff000000;
	    *call_addr |= (call_offset & 0x00ffffff);
	}
    }
}

extern char *
arm6_package_stitch(char *code, call_t *t, dill_pkg pkg)
{
    char *tmp = code;
    dill_lookup_xfer_addrs(t, &arm6_xfer_recs[0]);
    arm6_rt_call_link(code, t);
    return tmp + pkg->entry_offset;
}

