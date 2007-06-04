#include "config.h"
#include "dill.h"
#include "dill_internal.h"
#include "x86.h"

extern long dill_x86_hidden_mod(long a, long b)
{ return a % b; }
extern long dill_x86_hidden_umod(unsigned long a, unsigned long b)
{ return a % b; }

static xfer_entry x86_xfer_recs[3] = {
    {"dill_x86_hidden_mod", dill_x86_hidden_mod},
    {"dill_x86_hidden_umod", dill_x86_hidden_umod},
    {(char*)0, (void*)0}};

extern void
x86_rt_call_link(char *code, call_t *t)
{
    int i;

    for(i=0; i< t->call_count; i++) {
	int *call_addr = (int*) (code + t->call_locs[i].loc + 1);
	int call_offset = (int)(((char*)t->call_locs[i].xfer_addr) - 
	    ((char*)call_addr + 4));  /* add len of call insn */

	*call_addr = call_offset;
    }
}

extern char *
x86_package_stitch(char *code, call_t *t)
{
    dill_lookup_xfer_addrs(t, &x86_xfer_recs[0]);
    x86_rt_call_link(code, t);
    return code;
}
