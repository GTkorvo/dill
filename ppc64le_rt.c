#include "config.h"
#include "dill.h"
#include "dill_internal.h"
#include "ppc64le.h"

extern long dill_ppc64le_hidden_modi(int a, int b)
{ return a % b; }
extern long dill_ppc64le_hidden_mod(long a, long b)
{ return a % b; }
extern unsigned long dill_ppc64le_hidden_umod(unsigned long a, unsigned long b)
{ return a % b; }
extern unsigned int dill_ppc64le_hidden_umodi(unsigned int a, unsigned int b)
{ return a % b; }
extern double dill_ppc64le_hidden_ultod(unsigned long a)
{ return (double) a; }
extern float dill_ppc64le_hidden_ultof(unsigned long a)
{ return (float) a; }
extern unsigned long dill_ppc64le_hidden_dtoul(double a)
{ return (unsigned long) a; }
extern unsigned int dill_ppc64le_hidden_dtou(double a)
{ return (unsigned int) a; }
extern unsigned long dill_ppc64le_hidden_ftoul(float a)
{ return (unsigned long) a; }
extern unsigned int dill_ppc64le_hidden_ftou(float a)
{ return (unsigned int) a; }
extern long dill_ppc64le_hidden_udiv(unsigned long a, unsigned long b)
{ return a / b; }

static xfer_entry ppc64le_xfer_recs[] = {
    {"dill_ppc64le_hidden_modi", (void*)dill_ppc64le_hidden_modi},
    {"dill_ppc64le_hidden_mod", (void*)dill_ppc64le_hidden_mod},
    {"dill_ppc64le_hidden_umodi", (void*)dill_ppc64le_hidden_umodi},
    {"dill_ppc64le_hidden_umod", (void*)dill_ppc64le_hidden_umod},
    {"dill_ppc64le_hidden_ultod", (void*)dill_ppc64le_hidden_ultod},
    {"dill_ppc64le_hidden_ultof", (void*)dill_ppc64le_hidden_ultof},
    {"dill_ppc64le_hidden_dtoul", (void*)dill_ppc64le_hidden_dtoul},
    {"dill_ppc64le_hidden_ftoul", (void*)dill_ppc64le_hidden_ftoul},
    {"dill_ppc64le_hidden_dtou", (void*)dill_ppc64le_hidden_dtou},
    {"dill_ppc64le_hidden_ftou", (void*)dill_ppc64le_hidden_ftou},
    {"dill_ppc64le_hidden_udiv", (void*)dill_ppc64le_hidden_udiv},
    {(char*)0, (void*)0}};

#define ppc64le_sethi(r, imm) (HDR(0)|RD(r)|OP2(0x4)|imm)
#define ppc64le_ori(dest, src, imm) (HDR(0x2)|OP3(0x2)|IM|RD(dest)|RS1(src)|imm)
#define ppc64le_or(dest, src1, src2) (HDR(0x2)|OP3(0x2)|RD(dest)|RS1(src1)|RS2(src2))
#define ppc64le_sllx(dest, src1,imm) (HDR(0x2)|OP3(0x25)|RD(dest)|RS1(src1)|IM|SIMM13(imm)|(1<<12) )

extern void
ppc64le_rt_call_link(char *code, call_t *t, int force_plt)
{
    int i;

    for(i=0; i< t->call_count; i++) {
	int *call_addr = (int*) (code + t->call_locs[i].loc);
	if (!force_plt && (t->call_locs[i].mach_info == (void*)0)) {
	    /* no PLT */
	    int call_offset = (unsigned long)t->call_locs[i].xfer_addr -
		(unsigned long)call_addr;
	
	    *call_addr |= (call_offset & 0x03fffffc);
	} else {
	    int *plt = (int*)(code + t->call_locs[i].loc);
	    union {
		void *xa;
		long xl;
	    }u;
	    u.xa = t->call_locs[i].xfer_addr;
	    /* plt[0] = ppc64le_sethi(_g1, hh(u.xl)); */
	    /* plt[1] = ppc64le_ori(_g1, _g1, hm(u.xl)); */
	    /* plt[2] = ppc64le_sethi(_l0, lm(u.xl)); */
	    /* plt[3] = ppc64le_ori(_l0, _l0, lo(u.xl)); */
	    /* plt[4] = ppc64le_sllx(_g1, _g1, 32); */
	    /* plt[5] = ppc64le_or(_g1, _g1, _l0); */
	}
    }
}

static void
ppc64le_flush(void *base, void *limit)
{
#if defined(HOST_PPC64LE) || defined(HOST_PPC64LEV9)
    {
	volatile void *ptr = base;

#ifdef __GNUC__
	/* flush every 8 bytes of preallocated insn stream. */
	while((char*)ptr < (char*) limit) {
	  //	    asm volatile ("iflush %0" : /* */ : "r" (ptr));
	    ptr = (char *)ptr + 8;
	}
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
#else
	int nbytes = (char*)limit - (char*)base;
	for(; nbytes > 0;nbytes -= 8) {
	    asm("add %i0, 8, %i0");
	    asm ("iflush %i0");
	}

	asm ("nop");
	asm ("nop");
	asm ("nop");
	asm ("nop");
	asm ("nop");
#endif
#ifdef USE_MEMBAR
	asm("membar #Sync");
#endif
    }
#endif
}    

extern char *
ppc64le_package_stitch(char *code, call_t *t, dill_pkg pkg)
{
    int force_plt = 0;
    dill_lookup_xfer_addrs(t, &ppc64le_xfer_recs[0]);
#if defined(HOST_PPC64LEV9)
    force_plt = 1;
#endif
    ppc64le_rt_call_link(code, t, force_plt);
    ppc64le_flush(code, code + pkg->code_size);
    return code + pkg->entry_offset;
}
