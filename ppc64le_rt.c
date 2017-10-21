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

#define ppc64le_ori(s, dest, src, imm) 	INSN_OUT(s, D_FORM(24, dest, src, lo16(imm)));
#define ppc64le_andi(s, dest, src, imm) INSN_OUT(s, D_FORM(28, dest, src, lo16(imm)));
#define ppc64le_or(s, dest, src1, src2) INSN_OUT(s, X_FORM(31,src1, dest, src2, 444))
#define ppc64le_lshi(s, dest, src,imm) INSN_OUT(s, MD_FORM(30,dest,src,imm & 0x1f,63-imm, 0, imm>>5));

#define lo16(im) (((long)im) & 0xffff)
#define hi16(im) ((((long)im) & 0xffffffff) >> 16)
#define lo32(im) (((long)im) & 0xfffffffff)
#define hi32(im) ((((long)im) >> 32) &  0xfffffffff)

extern void
ppc64le_rt_call_link(char *code, call_t *t, int force_plt)
{
    int i;

    for(i=0; i< t->call_count; i++) {
	int *call_addr = (int*) (code + t->call_locs[i].loc);
	long xfer_addr = (unsigned long)t->call_locs[i].xfer_addr;
	call_addr[0] = D_FORM(15, _gpr12, _gpr0, hi16(hi32(xfer_addr)));
	call_addr[1] = D_FORM(24, _gpr12, _gpr12, lo16(hi32(xfer_addr)));
	call_addr[2] = MD_FORM(30, _gpr12, _gpr12, (32&0x1f), ((((63-32)&0x1f) << 1) | ((63-32)>>5)), 1, (32>>5));
	call_addr[3] = D_FORM(25, _gpr12, _gpr12, hi16(lo32(xfer_addr)));
	call_addr[4] = D_FORM(24, _gpr12, _gpr12, lo16(lo32(xfer_addr)));
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
    ppc64le_rt_call_link(code, t, force_plt);
    ppc64le_flush(code, code + pkg->code_size);
    return code + pkg->entry_offset;
}
