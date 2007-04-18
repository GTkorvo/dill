#include "config.h"

#ifdef LINUX_KERNEL_MODULE
#ifndef MODULE
#define MODULE
#endif
#ifndef __KERNEL__
#define __KERNEL__
#endif
#include <linux/kernel.h>
#include <linux/module.h>
#endif

#include "assert.h"

#ifndef LINUX_KERNEL_MODULE
#include <stdio.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#include <string.h>
#else
#include "kdill.h"
#define fprintf(fmt, args...)	printk(args)
#define printf	printk
#define malloc (void *)DAllocMM
#define free(a) DFreeMM((addrs_t) a)
#endif

#ifdef KPLUGINS_INTEGRATION
int dill_kplugins_integration = 1;
#else
int dill_kplugins_integration = 0;
#endif
/*
 * GANEV: note that we have to include "x86_64.h" _after_ including
 * "kdill.h" because it needs to #undef and then re-#define a few
 * symbols (namely, EAX, EBX, etc. defined in <asm/ptrace.h>
 */
#include "dill.h"
#include "dill_internal.h"
#include "x86_64.h"

#define MOV32 0x89
#define Mod(x) ((x)<<6)
#define RegOp(x) ((7&(x))<<3)
#define RM(x) (7&(x))
#define ModRM(mod,reg,rm) (Mod(mod)|RegOp(reg)|RM(rm))
#define SIB(scale, index, base) (((scale)<<6) | ((0x7 & (index))<<3) | (0x7 & (base)))

#define INSN_OUT(c, i) printf("Bad opout, line %d\n", __LINE__)
#define x86_64_savei(c, imm) 
#define x86_64_andi(c, dest, src, imm) x86_64_arith3i(c, 0x4, 0x2, dest, src, imm)

static void x86_64_push_reg(dill_stream c, int src);
static void x86_64_pop_reg(dill_stream c, int src);
#define x86_64_simple_ret(c) do {x86_64_pop_reg(c, R15);x86_64_pop_reg(c, R14); x86_64_pop_reg(c,R13); x86_64_pop_reg(c, R12);x86_64_pop_reg(c, EBX);x86_64_movl(c, ESP, EBP);x86_64_pop_reg(c, EBP); BYTE_OUT1(c, 0xc3);} while(0)
#define x86_64_nop(c) BYTE_OUT1(c, 0x90)

#define IREG 0
#define FREG 1

#define _temp_reg EAX
#define _frame_reg EBP
#define roundup(a,b) ((a + (b-1)) & (-b))
static void x86_64_pmov(dill_stream c, int typ, int dest, int src);

static 
struct basic_type_info 
{   char size;
    char align;
    char reg_type;
} type_info[] = {
    { 1, 1, IREG},  /* C */
    { 1, 1, IREG},  /* UC */
    { 2, 2, IREG},  /* S */
    { 2, 2, IREG},  /* US */
    { 4, 4, IREG},  /* I */
    { 4, 4, IREG},  /* U */
    { sizeof(long), sizeof(long), IREG},  /* UL */
    { sizeof(long), sizeof(long), IREG},  /* L */
    { sizeof(char*), sizeof(char*), IREG},  /* P */
    { sizeof(float), sizeof(float), FREG},  /* F */
    { sizeof(double), sizeof(double), FREG},  /* D */
    { 0, 8, IREG}, /* V */
    { -1, 8, IREG}, /* B */
    { sizeof(long), sizeof(long), IREG}, /* EC */
};

int x86_64_type_align[] = {
        1, /* C */
        1, /* UC */
        2, /* S */
        2, /* US */
        4, /* I */
        4, /* U */
        sizeof(unsigned long), /* UL */
        sizeof(long), /* L */
        sizeof(char*), /* P */
        4, /* F */
        4, /* D */
        4, /* V */
        4, /* B */
        sizeof(char*), /* EC */
};

int x86_64_type_size[] = {
        1, /* C */
        1, /* UC */
        2, /* S */
        2, /* US */
        4, /* I */
        4, /* U */
        sizeof(unsigned long), /* UL */
        sizeof(long), /* L */
        sizeof(char*), /* P */
        4, /* F */
        8, /* D */
        4, /* V */
        0, /* B */
        sizeof(char*), /* EC */
};

static void
BYTE_OUT2R(dill_stream c, int rex, int insn1, int insn2)
{
    unsigned char *tmp_ip;
    if (c->p->cur_ip >= c->p->code_limit) {
	extend_dill_stream(c);
    }
    tmp_ip = c->p->cur_ip;
    if (rex != 0) {
	*tmp_ip = (unsigned char)rex|0x40;
	*(tmp_ip + 1) = (unsigned char)insn1;
	*(tmp_ip + 2) = (unsigned char)insn2;
    } else {
	*(tmp_ip) = (unsigned char)insn1;
	*(tmp_ip + 1) = (unsigned char)insn2;
    }
    if (c->dill_debug) dump_cur_dill_insn(c);
    c->p->cur_ip = ((char*)c->p->cur_ip)+2;
    if (rex !=0) c->p->cur_ip++;
}

static void
BYTE_OUT1R(dill_stream c, int rex, int insn1)
{
    unsigned char *tmp_ip;
    if (c->p->cur_ip >= c->p->code_limit) {
	extend_dill_stream(c);
    }
    tmp_ip = c->p->cur_ip;
    if (rex != 0) {
	*tmp_ip = (unsigned char)rex|0x40;
	*(tmp_ip + 1) = (unsigned char)insn1;
    } else {
	*(tmp_ip) = (unsigned char)insn1;
    }
    if (c->dill_debug) dump_cur_dill_insn(c);
    c->p->cur_ip = ((char*)c->p->cur_ip)+1;
    if (rex !=0) c->p->cur_ip++;
}

static void
BYTE_OUT2IR(dill_stream c, int rex, int insn1, int insn2, int imm32)
{
    unsigned char *tmp_ip;
    if (c->p->cur_ip >= c->p->code_limit) {
	extend_dill_stream(c);
    }
    tmp_ip = c->p->cur_ip;
    if (rex != 0) {
	*tmp_ip = (unsigned char)rex|0x40;
	*(tmp_ip + 1) = (unsigned char)insn1;
	*(tmp_ip + 2) = (unsigned char)insn2;
	*((int *)(tmp_ip + 3)) = imm32;
    } else {
	*(tmp_ip) = (unsigned char)insn1;
	*(tmp_ip + 1) = (unsigned char)insn2;
	*((int *)(tmp_ip + 2)) = imm32;
    }
    if (c->dill_debug) dump_cur_dill_insn(c);
    c->p->cur_ip = ((char*)c->p->cur_ip)+6;
    if (rex !=0) c->p->cur_ip++;
}

static void
BYTE_OUT3IR(dill_stream c, int rex, int insn1, int insn2, int insn3, int imm32)
{
    unsigned char *tmp_ip;
    if (c->p->cur_ip >= c->p->code_limit) {
	extend_dill_stream(c);
    }
    tmp_ip = c->p->cur_ip;
    if (rex != 0) {
	*tmp_ip = (unsigned char)rex|0x40;
	*(tmp_ip + 1) = (unsigned char)insn1;
	*(tmp_ip + 2) = (unsigned char)insn2;
	*(tmp_ip + 3) = (unsigned char)insn3;
	*((int *)(tmp_ip + 4)) = imm32;
    } else {
	*(tmp_ip) = (unsigned char)insn1;
	*(tmp_ip + 1) = (unsigned char)insn2;
	*(tmp_ip + 2) = (unsigned char)insn3;
	*((int *)(tmp_ip + 3)) = imm32;
    }
    if (c->dill_debug) dump_cur_dill_insn(c);
    c->p->cur_ip = ((char*)c->p->cur_ip)+7;
    if (rex !=0) c->p->cur_ip++;
}

static void
BYTE_OUT1IR(dill_stream c, int rex, int insn1, int imm32)
{
    unsigned char *tmp_ip;
    if (c->p->cur_ip >= c->p->code_limit) {
	extend_dill_stream(c);
    }
    tmp_ip = c->p->cur_ip;
    if (rex != 0) {
	*tmp_ip = (unsigned char)rex|0x40;
	*(tmp_ip + 1) = (unsigned char)insn1;
	*((int *)(tmp_ip + 2)) = imm32;
    } else {
	*(tmp_ip) = (unsigned char)insn1;
	*((int *)(tmp_ip + 1)) = imm32;
    }
    if (c->dill_debug) dump_cur_dill_insn(c);
    c->p->cur_ip = ((char*)c->p->cur_ip)+5;
    if (rex !=0) c->p->cur_ip++;
}

static void
BYTE_OUT1LR(dill_stream c, int rex, int insn1, long imm64)
{
    unsigned char *tmp_ip;
    if (c->p->cur_ip >= c->p->code_limit) {
	extend_dill_stream(c);
    }
    tmp_ip = c->p->cur_ip;
    if (rex != 0) {
	*tmp_ip = (unsigned char)rex|0x40;
	*(tmp_ip + 1) = (unsigned char)insn1;
	*((long *)(tmp_ip + 2)) = imm64;
    } else {
	*(tmp_ip) = (unsigned char)insn1;
	*((long *)(tmp_ip + 1)) = imm64;
    }
    if (c->dill_debug) dump_cur_dill_insn(c);
    c->p->cur_ip = ((char*)c->p->cur_ip)+9;
    if (rex !=0) c->p->cur_ip++;
}

static void
BYTE_OUT3R(dill_stream c, int rex, int insn1, int insn2, int insn3)
{
    unsigned char *tmp_ip;
    if (c->p->cur_ip >= c->p->code_limit) {
	extend_dill_stream(c);
    }
    tmp_ip = c->p->cur_ip;
    if (rex != 0) {
	*tmp_ip = (unsigned char)rex|0x40;
	*(tmp_ip + 1) = (unsigned char)insn1;
	*(tmp_ip + 2) = (unsigned char)insn2;
	*(tmp_ip + 3) = (unsigned char)insn3;
    } else {
	*(tmp_ip) = (unsigned char)insn1;
	*(tmp_ip + 1) = (unsigned char)insn2;
	*(tmp_ip + 2) = (unsigned char)insn3;
    }
    if (c->dill_debug) dump_cur_dill_insn(c);
    c->p->cur_ip = ((char*)c->p->cur_ip)+3;
    if (rex !=0) c->p->cur_ip++;
}

static void
BYTE_OUT4R(dill_stream c, int rex, int insn1, int insn2, int insn3, int insn4)
{
    unsigned char *tmp_ip;
    if (c->p->cur_ip >= c->p->code_limit) {
	extend_dill_stream(c);
    }
    tmp_ip = c->p->cur_ip;
    if (rex != 0) {
	*tmp_ip = (unsigned char)rex|0x40;
	*(tmp_ip + 1) = (unsigned char)insn1;
	*(tmp_ip + 2) = (unsigned char)insn2;
	*(tmp_ip + 3) = (unsigned char)insn3;
	*(tmp_ip + 4) = (unsigned char)insn4;
    } else {
	*(tmp_ip) = (unsigned char)insn1;
	*(tmp_ip + 1) = (unsigned char)insn2;
	*(tmp_ip + 2) = (unsigned char)insn3;
	*(tmp_ip + 3) = (unsigned char)insn4;
    }
    if (c->dill_debug) dump_cur_dill_insn(c);
    c->p->cur_ip = ((char*)c->p->cur_ip)+4;
    if (rex !=0) c->p->cur_ip++;
}

static void
BYTE_OUT1R3I(dill_stream c, int insn1, int rex, int insn2, int insn3, int insn4, int imm)
{
    unsigned char *tmp_ip;
    if (c->p->cur_ip >= c->p->code_limit) {
	extend_dill_stream(c);
    }
    tmp_ip = c->p->cur_ip;
    if (rex != 0) {
	*(tmp_ip + 0) = (unsigned char)insn1;
	*(tmp_ip + 1) = (unsigned char)rex|0x40;
	*(tmp_ip + 2) = (unsigned char)insn2;
	*(tmp_ip + 3) = (unsigned char)insn3;
	*(tmp_ip + 4) = (unsigned char)insn4;
	*((int*)(tmp_ip + 5)) = imm;
    } else {
	*(tmp_ip) = (unsigned char)insn1;
	*(tmp_ip + 1) = (unsigned char)insn2;
	*(tmp_ip + 2) = (unsigned char)insn3;
	*(tmp_ip + 3) = (unsigned char)insn4;
	*((int*)(tmp_ip + 4)) = imm;
    }
    if (c->dill_debug) dump_cur_dill_insn(c);
    c->p->cur_ip = ((char*)c->p->cur_ip)+8;
    if (rex !=0) c->p->cur_ip++;
}

static void
BYTE_OUT1R4I(dill_stream c, int insn1, int rex, int insn2, int insn3, int insn4, int insn5, int imm)
{
    unsigned char *tmp_ip;
    if (c->p->cur_ip >= c->p->code_limit) {
	extend_dill_stream(c);
    }
    tmp_ip = c->p->cur_ip;
    if (rex != 0) {
	*(tmp_ip + 0) = (unsigned char)insn1;
	*(tmp_ip + 1) = (unsigned char)rex|0x40;
	*(tmp_ip + 2) = (unsigned char)insn2;
	*(tmp_ip + 3) = (unsigned char)insn3;
	*(tmp_ip + 4) = (unsigned char)insn4;
	*(tmp_ip + 5) = (unsigned char)insn5;
	*((int*)(tmp_ip + 6)) = imm;
    } else {
	*(tmp_ip) = (unsigned char)insn1;
	*(tmp_ip + 1) = (unsigned char)insn2;
	*(tmp_ip + 2) = (unsigned char)insn3;
	*(tmp_ip + 3) = (unsigned char)insn4;
	*(tmp_ip + 4) = (unsigned char)insn5;
	*((int*)(tmp_ip + 5)) = imm;
    }
    if (c->dill_debug) dump_cur_dill_insn(c);
    c->p->cur_ip = ((char*)c->p->cur_ip)+9;
    if (rex !=0) c->p->cur_ip++;
}

static void
BYTE_OUT1R3(dill_stream c, int insn1, int rex, int insn2, int insn3, int insn4)
{
    unsigned char *tmp_ip;
    if (c->p->cur_ip >= c->p->code_limit) {
	extend_dill_stream(c);
    }
    tmp_ip = c->p->cur_ip;
    if (rex != 0) {
	*(tmp_ip + 0) = (unsigned char)insn1;
	*(tmp_ip + 1)= (unsigned char)rex|0x40;
	*(tmp_ip + 2) = (unsigned char)insn2;
	*(tmp_ip + 3) = (unsigned char)insn3;
	*(tmp_ip + 4) = (unsigned char)insn4;
    } else {
	*(tmp_ip) = (unsigned char)insn1;
	*(tmp_ip + 1) = (unsigned char)insn2;
	*(tmp_ip + 2) = (unsigned char)insn3;
	*(tmp_ip + 3) = (unsigned char)insn4;
    }
    if (c->dill_debug) dump_cur_dill_insn(c);
    c->p->cur_ip = ((char*)c->p->cur_ip)+4;
    if (rex !=0) c->p->cur_ip++;
}

static void
BYTE_OUT1R4(dill_stream c, int insn1, int rex, int insn2, int insn3, int insn4, int insn5)
{
    unsigned char *tmp_ip;
    if (c->p->cur_ip >= c->p->code_limit) {
	extend_dill_stream(c);
    }
    tmp_ip = c->p->cur_ip;
    if (rex != 0) {
	*(tmp_ip + 0) = (unsigned char)insn1;
	*(tmp_ip + 1)= (unsigned char)rex|0x40;
	*(tmp_ip + 2) = (unsigned char)insn2;
	*(tmp_ip + 3) = (unsigned char)insn3;
	*(tmp_ip + 4) = (unsigned char)insn4;
	*(tmp_ip + 5) = (unsigned char)insn5;
    } else {
	*(tmp_ip) = (unsigned char)insn1;
	*(tmp_ip + 1) = (unsigned char)insn2;
	*(tmp_ip + 2) = (unsigned char)insn3;
	*(tmp_ip + 3) = (unsigned char)insn4;
	*(tmp_ip + 4) = (unsigned char)insn5;
    }
    if (c->dill_debug) dump_cur_dill_insn(c);
    c->p->cur_ip = ((char*)c->p->cur_ip)+5;
    if (rex !=0) c->p->cur_ip++;
}

static void
BYTE_OUT1R5(dill_stream c, int insn1, int rex, int insn2, int insn3, int insn4, int insn5, int insn6)
{
    unsigned char *tmp_ip;
    if (c->p->cur_ip >= c->p->code_limit) {
	extend_dill_stream(c);
    }
    tmp_ip = c->p->cur_ip;
    if (rex != 0) {
	*(tmp_ip + 0) = (unsigned char)insn1;
	*(tmp_ip + 1)= (unsigned char)rex|0x40;
	*(tmp_ip + 2) = (unsigned char)insn2;
	*(tmp_ip + 3) = (unsigned char)insn3;
	*(tmp_ip + 4) = (unsigned char)insn4;
	*(tmp_ip + 5) = (unsigned char)insn5;
	*(tmp_ip + 6) = (unsigned char)insn6;
    } else {
	*(tmp_ip) = (unsigned char)insn1;
	*(tmp_ip + 1) = (unsigned char)insn2;
	*(tmp_ip + 2) = (unsigned char)insn3;
	*(tmp_ip + 3) = (unsigned char)insn4;
	*(tmp_ip + 4) = (unsigned char)insn5;
	*(tmp_ip + 5) = (unsigned char)insn6;
    }
    if (c->dill_debug) dump_cur_dill_insn(c);
    c->p->cur_ip = ((char*)c->p->cur_ip)+6;
    if (rex !=0) c->p->cur_ip++;
}

#define x86_64_movi(c, dest, src)  x86_64_pmov(c, DILL_I, dest, src)
#define x86_64_movl(c, dest, src)  x86_64_pmov(c, DILL_L, dest, src)
#define x86_64_movd(c, dest, src)  x86_64_pmov(c, DILL_D, dest, src)
#define x86_64_movf(c, dest, src)  x86_64_pmov(c, DILL_F, dest, src)

static void x86_64_push_reg(dill_stream c, int src)
{
    int rex = 0;
    if (src > RDI) rex|= REX_B;
    BYTE_OUT1R(c, rex, 0x50+(src&0x7));
}

static void x86_64_pop_reg(dill_stream c, int src)
{
    int rex = 0;
    if (src > RDI) rex|= REX_B;
    BYTE_OUT1R(c, rex, 0x58+(src&0x7));
}


static void
x86_64_rshai(dill_stream c, int dest, int src, int imm) 
{
    int rex = REX_W;
    if (dest !=src) {
        x86_64_movl(c, dest, src);
    }
    if (dest > RDI) rex |= REX_B;
    BYTE_OUT3R(c, rex, 0xc1, ModRM(0x3, 0x7, dest), imm & 0xff);
}

static void
x86_64_rshi(dill_stream c, int dest, int src, int imm) 
{
    int rex = REX_W;
    if (dest !=src) {
        x86_64_movl(c, dest, src); 
    }
    if (dest > RDI) rex |= REX_B;
    BYTE_OUT3R(c, rex, 0xc1, ModRM(0x3, 0x5, dest), imm & 0xff);
}

static void
x86_64_lshi(dill_stream c, int dest, int src, int imm) 
{
    int rex = REX_W;
    if (dest != src) {
        x86_64_movl(c, dest, src);
    }
    if (dest > RDI) rex |= REX_B;
    BYTE_OUT3R(c, rex, 0xc1, ModRM(0x3, 0x4, dest), imm & 0xff);
}

static void
x86_64_pmov(dill_stream c, int typ, int dest, int src)
{
    int rex = 0;
    if ((typ == DILL_L) || (typ == DILL_UL) || (typ == DILL_P)) rex = REX_W;
    if (src > RDI) rex |= REX_R;
    if (dest > RDI) rex |= REX_B;
    if (src != dest) {
	switch (typ) {
	case DILL_D: case DILL_F:
	    BYTE_OUT3R(c, rex, 0x0f, 0x29, ModRM(0x3, src, dest));
	    break;
	default:
	    BYTE_OUT2R(c, rex, MOV32, ModRM(0x3, src, dest));
	    break;
	}
    }
}

extern void x86_64_farith2(c, b1, typ, dest, src)
dill_stream c;
int b1;
int typ;
int dest;
int src;
{
    /* this is fneg */
    int rex = 0;
    int op = 0xf3;
    if (src > XMM7) rex |= REX_B;
    if (dest > XMM7) rex |= REX_R;
    
    /* clear dest */
    {
	int rex1 = 0;
	if (dest > XMM7) rex1 = REX_R|REX_B;
	BYTE_OUT3R(c, rex, 0x0f, 0x57, ModRM(0x3, dest, dest));
    }
    if (typ == DILL_D) op = 0xf2;
    BYTE_OUT1R3(c, op, rex, 0x0f, 0x5c, ModRM(0x3, dest, src));
}

extern void x86_64_farith(c, b1, typ, dest, src1, src2)
dill_stream c;
int b1;
int typ;
int dest;
int src1;
int src2;
{
    int rex = 0;
    int op = 0xf3;
    
    if (src2 == dest) {
	/* protect src2 from overwrite (below) */
	x86_64_movd(c, XMM0, src2);
	src2 = XMM0;
    }
    if (src2 > XMM7) rex |= REX_B;
    if (dest > XMM7) rex |= REX_R;
    if (src1 != dest) x86_64_movd(c, dest, src1);
    if (typ == DILL_D) op = 0xf2;
    BYTE_OUT1R3(c, op, rex, 0x0f, b1, ModRM(0x3, dest, src2));
}

static void
x86_64_seti(dill_stream c, int r, int val) 
{
    int rex = 0;
    if (r > RDI) rex |= REX_B;
    BYTE_OUT1IR(c, rex, 0xb8 + (0x7 & r), val);
}

static void 
x86_64_setl(dill_stream c, int r, long val)
{
    int rex = REX_W;
    if (r > RDI) rex |= REX_B;
    BYTE_OUT1LR(c, rex, 0xb8 + ((0x7) & r), val);
}

extern int
x86_64_local(dill_stream c, int type)
{
    x86_64_mach_info smi = (x86_64_mach_info) c->p->mach_info;

    smi->act_rec_size += roundup(type_info[type].size, smi->stack_align);
    return (-smi->act_rec_size) + smi->stack_constant_offset;
}

extern int
x86_64_localb(dill_stream c, int size)
{
    x86_64_mach_info smi = (x86_64_mach_info) c->p->mach_info;
    smi->act_rec_size = roundup(smi->act_rec_size, size);

    smi->act_rec_size += roundup(size, smi->stack_align);
    return (-smi->act_rec_size) + smi->stack_constant_offset;
}

extern int x86_64_local_op(dill_stream c, int flag, int val)
{
    int size = val;
    if (flag == 0) {
	size = type_info[val].size;
    }
    return x86_64_localb(c, size);
}	

#define BEGIN_FLOAT_SAVE 64
extern void
x86_64_save_restore_op(dill_stream c, int save_restore, int type, int reg)
{
    x86_64_mach_info smi = (x86_64_mach_info) c->p->mach_info;
    int offset = 0;
    switch (type) {
    case DILL_D: case DILL_F:
	offset = reg * smi->stack_align + BEGIN_FLOAT_SAVE;
	break;
    default:
	if ((reg == RBX) || ((reg >= R12) && (reg <= R15))) {
	    /* RBX, R12-R15 inclusive are callee-saved, caller doesn't save */
	    return;
	}
	switch (reg) {
	case RDI:
	    offset = 0;
	    break;
	case RSI:
	    offset = 8;
	    break;
	case RDX:
	    offset = 16;
	    break;
	case RCX:
	    offset = 24;
	    break;
	case R8:
	    offset = 32;
	    break;
	case R9:
	    offset = 40;
	    break;
	case R10:
	    offset = 48;
	    break;
	case R11:
	    offset = 56;
	    break;
	default:
	    printf("x86_64 save_restore called with int reg %d\n", reg);
	    assert(0);  /* shouldn't happen */
	}
    }
    if (save_restore == 0) { /* save */
	x86_64_pstorei(c, type, 0, reg, _frame_reg, smi->save_base + offset);
    } else {  /* restore */
	x86_64_ploadi(c, type, 0, reg, _frame_reg, smi->save_base + offset);
    }
}	

/*
 * register   use  			       preserved across function calls
 *  %rax temporary register; with variable arguments
 *        passes information about the
 *	  number of SSE registers used; 1st return
 *	  register				            	No
 *  %rbx callee-saved register; optionally used
 *        as base pointer				    	Yes
 *  %rcx used to pass 4th integer argument to
 *	  functions					    	No
 *  %rdx used to pass 3rd argument to functions;
 *	  2nd return register				    	No
 *  %rsp stack pointer 					    	Yes
 *  %rbp callee-saved register; optionally used
 *       as frame pointer				    	Yes
 *  %rsi used to pass 2nd argument to functions 		No
 *  %rdi used to pass 1st argument to functions			No
 *  %r8  used to pass 5th argument to functions 		No
 *  %r9  used to pass 6th argument to functions			No
 *  %r10 temporary register, used for passing a 
 *       function?s static chain pointer 			No
 * %r11 temporary register					No
 * %r12-r15    callee-saved registers				Yes
 * %xmm0-%xmm1 used to pass and return floating point 
 * 			arguments 				No
 * %xmm2-%xmm7 used to pass floating point arguments		No
 * %xmm8-%xmm15 temporary registers 				No
 * %mmx0-%mmx7 temporary registers 				No
 * %st0 temporary register; used to return long double args	No
 * %st1 temporary registers; used to return long double args 	No
 * %st2-%st7 temporary registers 				No
 * %fs Reserved for system use (as thread specific data register) 

Figure 3.33: Register Save Area   (modified -- GSE )
Register Offset
%rdi 0
%rsi 8
%rdx 16
%rcx 24
%r8 32
%r9 40
%r10  48
%r11  56
%xmm0 64	BEGIN_FLOAT_SAVE
%xmm1 68
. . .
%xmm15 288

*/
static int arg_regs[] = {RDI, RSI, RDX, RCX, R8, R9};


extern void
x86_64_proc_start(dill_stream c, char *subr_name, int arg_count, arg_info_list args,
	     dill_reg *arglist)
{
    int i, int_arg_count, float_arg_count;
    int cur_arg_offset = 0;

    x86_64_mach_info smi = (x86_64_mach_info) c->p->mach_info;
    smi->pending_prefix = 0;
    x86_64_push_reg(c, EBP);
    x86_64_movl(c, EBP, ESP);
    smi->backpatch_offset = (char*)c->p->cur_ip - (char*)c->p->code_base;

    /* make local space reservation constant big so we have a word to patch */
    /* use the nop op code so that if we don't use all of it we get nops */
    dill_subli(c, ESP, ESP, 0x70909090);

    x86_64_push_reg(c, EBX);   /* callee is supposed to save these */
    x86_64_push_reg(c, R12);
    x86_64_push_reg(c, R13);
    x86_64_push_reg(c, R14);
    x86_64_push_reg(c, R15);

    /* leave some space */ x86_64_local(c, DILL_D);
    smi->conversion_word = x86_64_local(c, DILL_D);
    smi->fcu_word = x86_64_local(c, DILL_I);
    smi->save_base = x86_64_localb(c, BEGIN_FLOAT_SAVE + 16*16);

    cur_arg_offset = 16;
    int_arg_count = 0;
    float_arg_count = 0;
    for (i = 0; i < arg_count; i++) {
	if ((args[i].type != DILL_F) && (args[i].type != DILL_D) && (int_arg_count < 6)) {
	    args[i].is_register = 1;
	    args[i].offset = smi->save_base + int_arg_count*8;
	    args[i].in_reg = arg_regs[int_arg_count++];
	    args[i].out_reg = -1;
	} else if (((args[i].type == DILL_F) || (args[i].type == DILL_D)) && (float_arg_count < 8)) {
	    args[i].is_register = 1;
	    args[i].offset = smi->save_base + BEGIN_FLOAT_SAVE + float_arg_count*8;
	    args[i].in_reg = float_arg_count++;
	    args[i].out_reg = -1;
	} else {
	    args[i].is_register = 0;
	    args[i].offset = cur_arg_offset;
	    cur_arg_offset += roundup(type_info[(int)args[i].type].size, smi->stack_align);
	}
    }
    int_arg_count = float_arg_count = 0;   /* reset */
    for (i = 0; i < arg_count; i++) {
	int tmp_reg;
	int try_to_get_a_reg = 0;
	switch (args[i].type) {
	case DILL_D: case DILL_F:
	    try_to_get_a_reg = (float_arg_count++ < 4);
	    break;
	default:
	    try_to_get_a_reg = (int_arg_count++ < 4);
	    break;
	}
	if (try_to_get_a_reg) {
	    if (!dill_raw_getreg(c, &tmp_reg, args[i].type, DILL_VAR)) {
		/* not enough regs for this, store it to the stack */
		int real_offset = args[i].offset;
		if (arglist != NULL) arglist[i] = -1;
		x86_64_pstorei(c, args[i].type, 0, args[i].in_reg, EBP, 
				    real_offset);
		args[i].in_reg = -1;
		args[i].out_reg = -1;
		args[i].is_register = 0;
		continue;
	    }
	    if (args[i].is_register) {
		if ((args[i].type != DILL_F) && (args[i].type != DILL_D)) {
		    x86_64_movl(c, tmp_reg, args[i].in_reg);
		} else {	    /* must be float */
		    x86_64_movd(c, tmp_reg, args[i].in_reg);
		}
	    } else {
		/* general offset from fp*/
		int real_offset = args[i].offset; 
		x86_64_ploadi(c, args[i].type, 0, tmp_reg, EBP, real_offset);
	    }
	    if (arglist != NULL) arglist[i] = tmp_reg;
	    args[i].in_reg = tmp_reg;
	    args[i].is_register = 1;
	} else {
	    if (args[i].is_register) {
		/* store it to the stack */
	    } else {
		/* leave it on the stack */
		if (arglist != NULL) arglist[i] = -1;
		args[i].in_reg = -1;
		args[i].out_reg = -1;
	    }
	}
    }
}

static unsigned char ld_opcodes[] = {
    0x8a, /* DILL_C */
    0x8a, /* DILL_UC */
    0x8b, /* DILL_S */
    0x8b, /* DILL_US */
    0x8b, /* DILL_I */
    0x8b, /* DILL_U */
    0x8b, /* DILL_L */
    0x8b, /* DILL_UL */
    0x8b, /* DILL_P */
    0xd9, /* DILL_F */
    0xdd, /* DILL_D */
    0x00, /* DILL_V */
    0x00, /* DILL_B */
    0x8b, /* DILL_EC */
};

static void x86_64_clear(c, dest)
dill_stream c;
int dest;
{
    int rex = REX_W;
    if (dest > RDI) rex |= REX_B | REX_R;
    BYTE_OUT2R(c, rex, 0x33, ModRM(0x3, dest, dest));  /* xor dest, dest */
}

extern void
x86_64_ploadi(dill_stream c, int type, int junk, int dest, int src, long offset)
{
    unsigned char opcode = ld_opcodes[type];
    int tmp_dest = dest;
    x86_64_mach_info smi = (x86_64_mach_info) c->p->mach_info;
    int rex = 0;
    int float_op = 0;
    if ((type == DILL_L) || (type == DILL_UL) || (type == DILL_P)) {
	rex = REX_W;
    }
    switch (type) {
    case DILL_F: 
	float_op = 0xf3;
	break;
    case DILL_D:
	float_op = 0xf2;
	break;
    case DILL_C:
    case DILL_UC:
	if (dest >= ESP) {
	    /* movb doesn't work for big regs, load to eax */
	    tmp_dest = EAX;
	}
	if (type == DILL_UC) {
	    /* clear high bytes */
	    if (src == tmp_dest) {
		/* don't destroy source */
		tmp_dest = EAX;
	    }
	    x86_64_clear(c, tmp_dest);
	}
	break;
    case DILL_S: case DILL_US:
	if (type == DILL_US) {
	    /* clear high bytes */
	    if (src == tmp_dest) {
		/* don't destroy source */
		tmp_dest = EAX;
	    }
	    x86_64_clear(c, tmp_dest);
	}
	BYTE_OUT1(c, 0x66);
	break;
    case DILL_L: case DILL_UL: case DILL_P:
    /* fall through */
    default:
	break;
    }
    if (smi->pending_prefix != 0) {
        BYTE_OUT1(c, smi->pending_prefix);
	smi->pending_prefix = 0;
    }
    if (src > RDI) rex |= REX_B;
    if (tmp_dest > RDI) rex |= REX_R;
    if (((0x7 & src) == RSP) &&
	(((offset & 0xffffffff80000000) == 0) ||
	 ((offset & 0xffffffff80000000) == 0xffffffff80000000))) {
	/* must use SIB because ModRM has a discontinuity */
	if (offset == 0) {
	    if (float_op != 0) {
		BYTE_OUT1R4(c, float_op, rex, 0x0f, 0x10, ModRM(0x0, tmp_dest, 0x4), SIB(0,4,src));
	    } else {
		BYTE_OUT3R(c, rex, opcode, ModRM(0x0, tmp_dest, 0x4),SIB(0,4,src));
	    }
	} else if (((long)offset <= 127) && ((long)offset > -128)) {
	    if (float_op != 0) {
		BYTE_OUT1R5(c, float_op, rex, 0x0f, 0x10, ModRM(0x1, tmp_dest, src), SIB(0,ESP,0x4),offset & 0xff);
	    } else {
		BYTE_OUT4R(c, rex, opcode, ModRM(0x1, tmp_dest, src), SIB(0,ESP,0x4), offset & 0xff);
	    }
	} else {
	    if (float_op != 0) {
		BYTE_OUT1R4I(c, float_op, rex, 0x0f, 0x10, ModRM(0x2, tmp_dest, 0x4), SIB(0,4,src), offset);
	    } else {
		BYTE_OUT3IR(c, rex, opcode, ModRM(0x2, tmp_dest, 0x4), SIB(0,4,src),offset);
	    }
	}
    } else {
	if ((offset == 0) && ((src &0x7) != 5)){  /* avoid discontinuity in ModRM */
	    if (float_op != 0) {
		BYTE_OUT1R3(c, float_op, rex, 0x0f, 0x10, ModRM(0x0, tmp_dest, src));
	    } else {
		BYTE_OUT2R(c, rex, opcode, ModRM(0x0, tmp_dest, src));
	    }
	} else if (((long)offset <= 127) && ((long)offset > -128)) {
	    if (float_op != 0) {
		BYTE_OUT1R4(c, float_op, rex, 0x0f, 0x10, ModRM(0x1, tmp_dest, src), offset & 0xff);
	    } else {
		BYTE_OUT3R(c, rex, opcode, ModRM(0x1, tmp_dest, src), offset & 0xff);
	    }
	} else if (((offset & 0xffffffff80000000) == 0) ||
		   ((offset & 0xffffffff80000000) == 0xffffffff80000000)) {
	    if (float_op != 0) {
		BYTE_OUT1R3I(c, float_op, rex, 0x0f, 0x10, ModRM(0x2, tmp_dest, src), offset);
	    } else {
		BYTE_OUT2IR(c, rex, opcode, ModRM(0x2, tmp_dest, src), offset);
	    }
	} else {
	    /* really big offset */
	    if (dest != src) {
		/* use dest since it'll be destroyed by the load */
		x86_64_setl(c, dest, offset);
		x86_64_pload(c, type, 0, dest, src, dest);
	    } else {
		/* destroy src, but since it's the same as dest, it's lost anyway */
		x86_64_arith3i(c, 0, DILL_L, src, src, offset);
		x86_64_ploadi(c, type, 0, dest, src, 0);
	    }	    
	}
    }
    switch(type){
    case DILL_C:
	x86_64_lshi(c, dest, tmp_dest, 24);
	x86_64_rshi(c, dest, dest, 24);
	break;
    case DILL_S:
	x86_64_lshi(c, dest, tmp_dest, 16);
	x86_64_rshi(c, dest, dest, 16);
	break;
    case DILL_UC: case DILL_US:
	if (dest != tmp_dest)
	    x86_64_movi(c, dest, tmp_dest);
	break;
    }
}

extern void
x86_64_pload(dill_stream c, int type, int junk, int dest, int src1, int src2)
{
    unsigned char opcode = ld_opcodes[type];
    int tmp_dest = dest;
    int clear_high_bytes_after_load = 0;

    x86_64_mach_info smi = (x86_64_mach_info) c->p->mach_info;
    int rex = 0;
    int float_op = 0;
    if ((type == DILL_L) || (type == DILL_UL) || (type == DILL_P)) {
	rex = REX_W;
    }
    switch(type){
    case DILL_C:
    case DILL_UC:
	if (dest >= ESP) {
	    /* movb doesn't work for big regs, load to eax */
	    tmp_dest = EAX;
	}
	if (type == DILL_UC) {
	    /* clear high bytes */
	    if ((src1 == tmp_dest) || (src2 == tmp_dest)) {
		/* don't destroy source */
		tmp_dest = EAX;
	    }
	    if ((src1 == EAX) || (src2 == EAX)) {
		clear_high_bytes_after_load = 1;
	    } else {
		x86_64_clear(c, tmp_dest);
	    }
	}
	break;
    case DILL_S: case DILL_US:
	if (type == DILL_US) {
	    /* clear high bytes */
	    if ((src1 == tmp_dest) || (src2 == tmp_dest)) {
		/* don't destroy source */
		tmp_dest = EAX;
	    }
	    if ((src1 == EAX) || (src2 == EAX)) {
		clear_high_bytes_after_load = 1;
	    } else {
		x86_64_clear(c, tmp_dest);
	    }
	}
	BYTE_OUT1(c, 0x66);
	break;
    case DILL_F:
	float_op = 0xf3;
	break;
    case DILL_D:
	float_op = 0xf2;
	break;
    case DILL_L: case DILL_UL: case DILL_P:
	if (smi->stack_align == 4) {
	    type = DILL_I;
	}
    }
    if (smi->pending_prefix != 0) {
        BYTE_OUT1(c, smi->pending_prefix);
	smi->pending_prefix = 0;
    }
    if ((src2 & 0x7) == 0x5) {   /* avoid SIB discontinuity */
	int tmp = src1;
	src1 = src2;
	src2 = tmp;
    }
    if (src1 > RDI) rex |= REX_X;
    if (src2 > RDI) rex |= REX_B;
    if (tmp_dest > RDI) rex |= REX_R;
    if (float_op != 0) {
	BYTE_OUT1R4(c, float_op, rex, 0x0f, 0x10, ModRM(0x0, tmp_dest, 0x4), SIB(0,src1,src2));
    } else {
	BYTE_OUT3R(c, rex, opcode, ModRM(0x0, tmp_dest, 0x4), SIB(0,src1,src2));
    }
    switch(type){
    case DILL_C:
	x86_64_lshi(c, dest, tmp_dest, 24);
	x86_64_rshi(c, dest, dest, 24);
	break;
    case DILL_S:
	x86_64_lshi(c, dest, tmp_dest, 16);
	x86_64_rshi(c, dest, dest, 16);
	break;
    case DILL_UC: case DILL_US:
	if (dest != tmp_dest) {
	    if (clear_high_bytes_after_load) {
		int bits = 56;
		if (type == DILL_US) bits = 48;
		x86_64_lshi(c, dest, tmp_dest, bits);
		x86_64_rshi(c, dest, dest, bits);
	    } else {
		x86_64_movi(c, dest, tmp_dest);
	    }
	}
	break;
    }
}

extern void
x86_64_pbsloadi(dill_stream c, int type, int junk, int dest, int src, long offset)
{
    int rex = 0;
    int fdest = dest;
    int ltype = type;
    switch (type) {
    case DILL_D:
	ltype = DILL_L;
	fdest = dest;
	dest = EAX;
	break;
    case DILL_F:
	ltype = DILL_I;
	fdest = dest;
	dest = EAX;
	break;
    }
    x86_64_ploadi(c, ltype, junk, dest, src, offset);
    if (dest > RDI) rex |= REX_B;
    switch(type) {
    case DILL_F:
	rex = 0;
	if (fdest > RDI) rex |= REX_R;
	x86_64_bswap(c, 0, DILL_I, EAX, EAX);
	BYTE_OUT1R3(c, 0x66, rex, 0x0f, 0x6e, ModRM(0x3, fdest, EAX));
	break;
    case DILL_D:
	rex = REX_W;
	if (fdest > RDI) rex |= REX_R;
	x86_64_bswap(c, 0, DILL_L, EAX, EAX);
	BYTE_OUT1R3(c, 0x66, rex, 0x0f, 0x6e, ModRM(0x3, fdest, EAX));
	break;
    case DILL_L: case DILL_UL: case DILL_P: 
	rex |= REX_W;
	/* falling through */
    case DILL_I: case DILL_U:
	BYTE_OUT2R(c, rex, 0x0f, 0xc8 + (dest&0x7));   /* byteswap dest */
	break;
    case DILL_S: case DILL_US:
	/* byteswap 32 bits and shift down 16 */
	BYTE_OUT2R(c, rex, 0x0f, 0xc8 + (dest&0x7));   /* byteswap dest */
	x86_64_rshi(c, dest, dest, 16);
	break;
    case DILL_C: case DILL_UC:
	break;
    }
}


extern void
x86_64_pbsload(dill_stream c, int type, int junk, int dest, int src1, int src2)
{
    int rex = 0;
    int fdest = dest;
    int ltype = type;
    switch (type) {
    case DILL_D:
	ltype = DILL_L;
	fdest = dest;
	dest = EAX;
	break;
    case DILL_F:
	ltype = DILL_I;
	fdest = dest;
	dest = EAX;
	break;
    }
    if (dest > RDI) rex |= REX_B;
    x86_64_pload(c, ltype, junk, dest, src1, src2);
    switch(type) {
    case DILL_F:
	rex = 0;
	x86_64_bswap(c, 0, DILL_I, EAX, EAX);
	if (fdest > RDI) rex |= REX_R;
	BYTE_OUT1R3(c, 0x66, rex, 0x0f, 0x6e, ModRM(0x3, fdest, EAX));
	break;
    case DILL_D:
	rex = REX_W;
	if (fdest > RDI) rex |= REX_R;
	x86_64_bswap(c, 0, DILL_L, EAX, EAX);
	BYTE_OUT1R3(c, 0x66, rex, 0x0f, 0x6e, ModRM(0x3, fdest, EAX));
	break;
    case DILL_L: case DILL_UL: case DILL_P: 
	rex |= REX_W;
    case DILL_I: case DILL_U:
	BYTE_OUT2R(c, rex, 0x0f, 0xc8 + (dest&0x7));   /* byteswap dest */
	break;
    case DILL_S: case DILL_US:
	/* byteswap 32 bits and shift down 16 */
	BYTE_OUT2R(c, rex, 0x0f, 0xc8 + (dest&0x7));   /* byteswap dest */
	x86_64_rshi(c, dest, dest, 16);
	break;
    case DILL_C: case DILL_UC:
	break;
    }
}

static unsigned char st_opcodes[] = {
    0x88, /* DILL_C */
    0x88, /* DILL_UC */
    0x89, /* DILL_S */
    0x89, /* DILL_US */
    0x89, /* DILL_I */
    0x89, /* DILL_U */
    0x89, /* DILL_L */
    0x89, /* DILL_UL */
    0x89, /* DILL_P */
    0xd9, /* DILL_F */
    0xdd, /* DILL_D */
    0x00, /* DILL_V */
    0x00, /* DILL_B */
    0x89, /* DILL_EC */
};
extern void
x86_64_pstorei(dill_stream c, int type, int junk, int dest, int src, long offset)
{
    x86_64_mach_info smi = (x86_64_mach_info) c->p->mach_info;
    int rex = 0, float_op = 0;
    if ((type == DILL_L) || (type == DILL_UL) || (type == DILL_P)) {
	rex = REX_W;
    }
    switch(type) {
    case DILL_C:    case DILL_UC:
	if (dest >= ESP) {
	    /* movb doesn't work for big regs, move to eax */
	    x86_64_movi(c, EAX, dest);
	    dest = EAX;
	}
	break;
    }
    switch (type) {
    case DILL_F:
	float_op = 0xf3;
	break;
    case DILL_D:
	float_op = 0xf2;
	break;
    case DILL_S: case DILL_US:
	BYTE_OUT1(c, 0x66);
	break;
    default:
	break;
    }
    if (smi->pending_prefix != 0) {
        BYTE_OUT1(c, smi->pending_prefix);
	smi->pending_prefix = 0;
    }
    if (dest > RDI) rex |= REX_R;
    if (src > RDI) rex |= REX_B;
    if (((src&0x7) == ESP) &&
	(((offset & 0xffffffff80000000) == 0) ||
	 ((offset & 0xffffffff80000000) == 0xffffffff80000000))) {
	if (((long)offset <= 127) && ((long)offset > -128)) {
	    if (float_op != 0) {
		BYTE_OUT1R5(c, float_op, rex, 0x0f, 0x11, ModRM(0x1, dest, src), SIB(0x0,ESP,0x4), offset & 0xff);
	    } else {
		BYTE_OUT4R(c, rex, st_opcodes[type], ModRM(0x1, dest, src), SIB(0x0,ESP,0x4), offset & 0xff);
	    }
	} else {
	    if (float_op != 0) {
		BYTE_OUT1R4I(c, float_op, rex, 0x0f, 0x11, ModRM(0x2, dest, 0x4), SIB(0,4,src), offset);
	    } else {
		BYTE_OUT3IR(c, rex, st_opcodes[type], ModRM(0x2, dest, 0x4), SIB(0,4,src),offset);
	    }
	}
    } else {
	if ((offset == 0) && ((src &0x7) != 5)){  /* avoid discontinuity in ModRM */
	    if (float_op != 0) {
		BYTE_OUT1R3(c, float_op, rex, 0x0f, 0x11, ModRM(0x0, dest, src));
	    } else {
		BYTE_OUT2R(c, rex, st_opcodes[type], ModRM(0x0, dest, src));
	    }
	} else if (((long)offset <= 127) && ((long)offset > -128)) {
	    if (float_op != 0) {
		BYTE_OUT1R4(c, float_op, rex, 0x0f, 0x11, ModRM(0x1, dest, src), offset & 0xff);
	    } else {
		BYTE_OUT3R(c, rex, st_opcodes[type], ModRM(0x1, dest, src), offset & 0xff);
	    }
	} else if (((offset & 0xffffffff80000000) == 0) ||
		   ((offset & 0xffffffff80000000) == 0xffffffff80000000)) {
	    /* safe INT offset using only low 31 bits */
	    if (float_op != 0) {
		BYTE_OUT1R3I(c, float_op, rex, 0x0f, 0x11, ModRM(0x2, dest, src), offset);
	    } else {
		BYTE_OUT2IR(c, rex, st_opcodes[type], ModRM(0x2, dest, src), offset);
	    }
	} else {
	    x86_64_push_reg(c, src);
	    x86_64_arith3i(c, 0, DILL_L, src, src, offset);
	    x86_64_pstorei(c, type, 0, dest, src, 0);
	    x86_64_pop_reg(c, src);
	}
    }
}

extern void
x86_64_pstore(dill_stream c, int type, int junk, int dest, int src1, int src2)
{
    x86_64_mach_info smi = (x86_64_mach_info) c->p->mach_info;
    int rex = 0, float_op = 0;
    if ((type == DILL_L) || (type == DILL_UL) || (type == DILL_P)) {
	rex = REX_W;
    }
    switch (type) {
    case DILL_F:
	float_op = 0xf3;
	break;
    case DILL_D:
	float_op = 0xf2;
	break;
    case DILL_S: case DILL_US:
	BYTE_OUT1(c, 0x66);
	break;
    default:
	break;
    }
    if (smi->pending_prefix != 0) {
        BYTE_OUT1(c, smi->pending_prefix);
	smi->pending_prefix = 0;
    }
    if ((src2 & 0x7) == 0x5) {   /* avoid SIB discontinuity */
	int tmp = src1;
	src1 = src2;
	src2 = tmp;
    }
    if (src1 > RDI) rex |= REX_X;
    if (src2 > RDI) rex |= REX_B;
    if (dest > RDI) rex |= REX_R;
    if (float_op != 0) {
	BYTE_OUT1R4(c, float_op, rex, 0x0f, 0x11, ModRM(0x0, dest, 0x4), SIB(0,src1,src2));
    } else {
	BYTE_OUT3R(c, rex, st_opcodes[type], ModRM(0x0, dest, 0x4), SIB(0,src1,src2));
    }
}

static long dill_hidden_mod(long a, long b)
{ return a % b; }
static long dill_hidden_umod(unsigned long a, unsigned long b)
{ return a % b; }
static double dill_hidden_ULtoD(unsigned long a)
{ return (double) a; }
static unsigned long dill_hidden_DtoUL(double a)
{ unsigned long l = a; return l; }

extern void x86_64_mod(dill_stream c, int data1, int data2, int dest, int src1, 
		      int src2)
{
    int return_reg;
    if (data1 == 1) {
	/* signed case */
	return_reg = dill_scalll(c, (void*)dill_hidden_mod, "%l%l", src1, src2);
	dill_movl(c, dest, return_reg);
    } else {
	/* unsigned case */
	return_reg = dill_scalll(c, (void*)dill_hidden_umod, "%l%l", src1, src2);
	dill_movul(c, dest, return_reg);
    }
}

extern void x86_64_modi(dill_stream c, int data1, int data2, int dest, int src1, 
		      long imm)
{
    x86_64_seti(c, _temp_reg, imm);
    x86_64_mod(c, data1, data2, dest, src1, _temp_reg);
}

extern void x86_64_div(dill_stream c, int op3, int op, int dest, int src1, 
		      int src2)
{
}

extern void x86_64_divi(dill_stream c, int op3, int op, int dest, int src, 
		      long imm)
{
}

extern void
x86_64_mov(dill_stream c, int type, int junk, int dest, int src)
{
    x86_64_pmov(c, type, dest, src);
}

extern void x86_64_arith3(c, op, typ, dest, src1, src2)
dill_stream c;
int op;
int typ;
int dest;
int src1;
int src2;
{
    int commut = ( op != 0x2b);   /* subtract not commutative */
    int rex = 0;
    if ((typ == DILL_L) || (typ == DILL_UL) || (typ == DILL_P)) {
	rex = REX_W;
    }

    if (commut && (dest == src1)) {
	if (dest > RDI) rex |= REX_R;
	if (src2 > RDI) rex |= REX_B;
	BYTE_OUT2R(c, rex, op, ModRM(0x3, dest, src2));
    } else if (commut && (dest == src2)) {
	if (dest > RDI) rex |= REX_R;
	if (src1 > RDI) rex |= REX_B;
	BYTE_OUT2R(c, rex, op, ModRM(0x3, dest, src1));
    } else if (dest == src2) {
	assert(op == 0x2b);	/* must be subtract */
	int rex1 = rex, rex2 = rex;
	if (dest > RDI) rex1 |= REX_B;
	BYTE_OUT2R(c, rex1, 0xf7, ModRM(0x3, 0x3, dest));   /* neg src2/dest */
	if (dest > RDI) rex2 |= REX_R;
	if (src1 > RDI) rex2 |= REX_B;
	BYTE_OUT2R(c, rex2, 0x03, ModRM(0x3, dest, src1));  /* add src1, dest */
    } else {
	int rex1 = rex, rex2 = rex;
	if (dest > RDI) rex1 |= REX_B;
	if (src1 > RDI) rex1 |= REX_R;
	BYTE_OUT2R(c, rex1, MOV32, ModRM(0x3, src1, dest));
	if (src2 > RDI) rex2 |= REX_B;
	if (dest > RDI) rex2 |= REX_R;
	BYTE_OUT2R(c, rex2, op, ModRM(0x3, dest, src2));
    }
}

extern void x86_64_arith2(c, op, subop, dest, src)
dill_stream c;
int op;
int subop;
int dest;
int src;
{
    if (op == 0) {
	int tmp_dest = dest;
	int rex1 = REX_W, rex2 = REX_W;
	/* must be not */
	if (dest >= ESP) {
	    tmp_dest = EAX;
	}
	if (src > RDI) rex1 |= REX_B;
	BYTE_OUT3R(c, rex1, 0x83, ModRM(0x3, 0x7, src), 0);  /* cmp */
	x86_64_seti(c, tmp_dest, 0);
	if (src > RDI) rex2 |= REX_R;
	if (tmp_dest > RDI) rex2 |= REX_B;
	BYTE_OUT3R(c, rex2, 0x0f, 0x94, ModRM(0x3, src, tmp_dest));  /* sete dest */
	if (tmp_dest != dest) {
	    x86_64_movi(c, dest, tmp_dest);
	}	    
    } else {
	int rex = REX_W;
	if (src != dest) {
	    int rex1 = rex;
	    if (dest > RDI) rex1 |= REX_B;
	    if (src > RDI) rex1 |= REX_R;
	    BYTE_OUT2R(c, rex1, MOV32, ModRM(0x3, src, dest));
	}
	if (dest > RDI) rex |= REX_B;
	BYTE_OUT2R(c, rex, op, ModRM(0x3, subop, dest));
    }
}

extern void x86_64_bswap(c, junk, typ, dest, src)
dill_stream c;
int junk;
int typ;
int dest;
int src;
{
    int rex = 0;
    switch(typ) {
    case DILL_F: 
	rex = 0;
	if (dest > RDI) rex |= REX_R;
	BYTE_OUT1R3(c, 0x66, rex, 0x0f, 0x7e, ModRM(0x3, src, EAX));
	x86_64_bswap(c, 0, DILL_I, EAX, EAX);
	BYTE_OUT1R3(c, 0x66, rex, 0x0f, 0x6e, ModRM(0x3, dest, EAX));
	break;
    case DILL_D:
	rex = REX_W;
	if (dest > RDI) rex |= REX_R;
	BYTE_OUT1R3(c, 0x66, rex, 0x0f, 0x7e, ModRM(0x3, src, EAX));
	x86_64_bswap(c, 0, DILL_L, EAX, EAX);
	BYTE_OUT1R3(c, 0x66, rex, 0x0f, 0x6e, ModRM(0x3, dest, EAX));
	break;
    case DILL_L: case DILL_UL: case DILL_P: 
	rex |= REX_W;
    case DILL_I: case DILL_U:
	if (dest > RDI) rex |= REX_B;
	if (dest != src) {
	    x86_64_movl(c, dest, src);
	}
	BYTE_OUT2R(c, rex, 0x0f, 0xc8 + (dest&0x7));   /* byteswap dest */
	break;
    case DILL_S: case DILL_US:
	if (dest > RDI) rex |= REX_B;
	if (dest != src) {
	    x86_64_movl(c, dest, src);
	}
	/* byteswap 32 bits and shift down 16 */
	BYTE_OUT2R(c, rex, 0x0f, 0xc8 + (dest&0x7));   /* byteswap dest */
	x86_64_rshi(c, dest, dest, 16);
	break;
    case DILL_C: case DILL_UC:
	break;
    }
}

extern void x86_64_mul(c, sign, imm, dest, src1, src2)
dill_stream c;
int sign;
int imm;
int dest;
int src1;
long src2;
{
    int rex = REX_W;
    /* make src1 be EAX */
    if (dest != EAX) {
	x86_64_push_reg(c, EAX);
    }
    if (dest != EDX) {
	x86_64_push_reg(c, EDX);
    }
	
    if ((src2 == EAX) && !imm){
	int tmp = src2;
	src1 = src2;
	src2 = tmp;
    }
    if (src1 != EAX) {
	x86_64_movl(c, EAX, src1);
    }
    if (imm == 0) {
	if (src2 > RDI) rex |= REX_B;
	BYTE_OUT2R(c, rex, 0xf7, ModRM(0x3, sign ? 0x5 : 0x4, src2));
    } else {
	/* src2 is really immediate */
	if (sign && ((src2 & 0xffffffff80000000) == 0)) {
	    BYTE_OUT2IR(c, rex, 0x69, ModRM(0x3, 0, EAX), src2);
	} else {
	    x86_64_setl(c, EDX, src2);
	    BYTE_OUT2R(c, rex, 0xf7, ModRM(0x3, 0x4, EDX));
	}
    }
    if (dest != EDX) {
	x86_64_pop_reg(c, EDX);
    }
    if (dest != EAX) {
	x86_64_movl(c, dest, EAX);
	x86_64_pop_reg(c, EAX);
    }
}

extern void x86_64_div_modi(c, div, type, dest, src1, imm)
dill_stream c;
int div;
int type;
int dest;
int src1;
long imm;
{
    x86_64_push_reg(c, EBP);
    x86_64_setl(c, EBP, imm);
    x86_64_div_mod(c, div, type, dest, src1, EBP);
    x86_64_pop_reg(c, EBP);
}

extern void x86_64_div_mod(c, div, type, dest, src1, src2)
dill_stream c;
int div;
int type;
int dest;
int src1;
int src2;
{
    int tmp_src2 = src2;
    int rex = 0;
    int sign = ((type == DILL_I) || (type == DILL_L));
    
    if ((type == DILL_UL) || (type == DILL_L)) {
	rex = REX_W;
    }
    /* make src1 be EAX */
    if (dest != EAX) {
	x86_64_push_reg(c, EAX);
    }
    if (dest != EDX) {
	x86_64_push_reg(c, EDX);
    }
	
    if (src1 != EAX) {
	x86_64_movl(c, EAX, src1);
    }
    if (src2 == EDX) {
	tmp_src2 = EBP;
	x86_64_push_reg(c, EBP);
	x86_64_movl(c, EBP, src2);
    }
    if (type == DILL_I) {
	BYTE_OUT1(c, 0x99);   /* cltd  (altern  cdq) */
/*	rex |= REX_W;*/
    } else if (sign) {
	x86_64_rshai(c, EDX, EAX, 63);
/*	x86_64_movl(c, EDX, EAX);*/
    } else {
	x86_64_clear(c, EDX);
    }
    if (tmp_src2 > RDI) rex |= REX_B;
    BYTE_OUT2R(c, rex, 0xf7, ModRM(0x3, sign ? 0x7 : 0x6, tmp_src2));
    if (src2 == EDX) {
	x86_64_pop_reg(c, EBP);
    }
    if (div && (dest != EAX)) {
	x86_64_movl(c, dest, EAX);
    }
    if (!div && (dest != EDX)) {
	x86_64_movl(c, dest, EDX);
    }
    if (dest != EDX) {
	x86_64_pop_reg(c, EDX);
    }
    if (dest != EAX) {
	x86_64_pop_reg(c, EAX);
    }
	
}

static int group1_eax_op[] = {
    0x05 /* add */,
    0x0d /* or */,
    0x15 /* adc */,
    0x1d /* sbb */,
    0x25 /* and */,
    0x2d /* sub */,
    0x35 /* xor */,
    0x3d /* cmp */
};

extern void x86_64_arith3i(c, op, typ, dest, src, imm)
dill_stream c;
int op;
int typ;
int dest;
int src;
long imm;
{
    int rex = 0;
    if ((typ == DILL_L) || (typ == DILL_UL) || (typ == DILL_P)) {
	rex = REX_W;
    }

    if (dest != src) {
	int rex1 = rex;
	if (dest > RDI) rex1 |= REX_B;
	if (src > RDI) rex1 |= REX_R;
	BYTE_OUT2R(c, rex1, MOV32, ModRM(0x3, src, dest));
    }
    if ((imm <= 127) && (imm > -128)) {
	int rex1 = rex;
	if (dest > RDI) rex1 |= REX_B;
	BYTE_OUT3R(c, rex1, 0x83, ModRM(0x3, op, dest), imm & 0xff);
	return;
    }
    if ((0xffffffff80000000 & imm) == 0) {
	/* int-sized immediate */
	if (dest == EAX) {
	    BYTE_OUT1IR(c, rex, group1_eax_op[op], imm);
	} else {
	    int rex2 = rex;
	    if (dest > RDI) rex2 |= REX_B;
	    BYTE_OUT2IR(c, rex2, 0x81, ModRM(0x3, op, dest), imm);
	}
    } else {
	int tmp_reg = dest;
	if (src == dest) {
	    if (src == EAX) {
		tmp_reg = R11;
	    } else {
		tmp_reg = EAX;
	    }
	    x86_64_push_reg(c, tmp_reg);
	}
	x86_64_pset(c, typ, 0, tmp_reg, imm);
	switch(op) {
	case 0: /* add */ op = 0x03;  /* reg to reg */ break;
	case 5: /* sub */ op = 0x2b;  /* reg to reg */ break;
	case 4: /* and */ op = 0x23;  /* reg to reg */ break;
	case 1: /* or  */ op = 0x0b;  /* reg to reg */ break;
	case 6: /* xor */ op = 0x33;  /* reg to reg */ break;
	}
	x86_64_arith3(c, op, typ, dest, src, tmp_reg);
	if (src == dest) {
	    x86_64_pop_reg(c, tmp_reg);
	}
    }
}

extern void x86_64_shift(c, op, type, dest, src1, src2)
dill_stream c;
int op;
int type;
int dest;
int src1;
int src2;
{
    int tmp_dest = dest;
    int rex = 0;
    if ((type == DILL_L) || (type == DILL_UL) || (type == DILL_P)) {
	rex = REX_W;
    }
    if ((dest == ECX) || (dest == src2)) {
	x86_64_push_reg(c, EAX);
	tmp_dest = EAX;
    }
    if (tmp_dest != src1) {
	x86_64_movl(c, tmp_dest, src1);
    }
    if (src2 != ECX) {
	x86_64_push_reg(c, ECX);
	x86_64_movl(c, ECX, src2);
    }
    if (tmp_dest > RDI) rex |= REX_B;
    BYTE_OUT2R(c, rex, 0xd3, ModRM(0x3, op, tmp_dest));
    if (src2 != ECX) {
	x86_64_pop_reg(c, ECX);
    }
    if ((dest == ECX) || (dest == src2)) {
	x86_64_movl(c, dest, tmp_dest);
	x86_64_pop_reg(c, EAX);
    }
}

extern void x86_64_shifti(c, op, type, dest, src, imm)
dill_stream c;
int op;
int type;
int dest;
int src;
long imm;
{
    int rex = 0;
    if ((type == DILL_L) || (type == DILL_UL) || (type == DILL_P)) {
	rex = REX_W;
    }
    if (dest != src) {
	x86_64_movl(c, dest, src);
    }
    if (dest > RDI) rex |= REX_B;
    BYTE_OUT3R(c, rex, 0xc1, ModRM(0x3, op, dest), imm & 0xff);
}

#define CONV(x,y) ((x*100)+y)
extern void
x86_64_convert(dill_stream c, int from_type, int to_type, 
	      int dest, int src)
{
    switch(CONV(from_type, to_type)) {
    case CONV(DILL_I, DILL_L):
    case CONV(DILL_L,DILL_I):
    case CONV(DILL_I,DILL_UL):
	/* signext24 - lsh32, rsha32 */
	x86_64_lshi(c, dest, src, 32);
	x86_64_rshai(c, dest, dest, 32);
	break;
    case CONV(DILL_U,DILL_I):
    case CONV(DILL_U,DILL_UL):
    case CONV(DILL_U,DILL_L):
    case CONV(DILL_I, DILL_U):
    case CONV(DILL_UL,DILL_U):
    case CONV(DILL_L,DILL_U):
	/* clear upper 32 - lsh32, rsh32 */
	x86_64_lshi(c, dest, src, 32);
	x86_64_rshi(c, dest, dest, 32);
	break;
    case CONV(DILL_UL,DILL_I):
    case CONV(DILL_UL,DILL_L):
    case CONV(DILL_L,DILL_UL):
    case CONV(DILL_P,DILL_UL):
    case CONV(DILL_UL,DILL_P):
	if(src == dest) return;
	x86_64_movl(c, dest,src);
	break;
    case CONV(DILL_D,DILL_F):
    case CONV(DILL_F,DILL_D):
	/* cvtss2sd */
	/* cvtsd2ss */
    {
	int rex = REX_W;
	/* cvtsi2s{s,d} */
	if (src > RDI) rex |= REX_B;
	if (dest > RDI) rex |= REX_R;
	BYTE_OUT1R3(c, (from_type == DILL_D) ? 0xf2 : 0xf3, rex, 0xf, 0x5a, ModRM(0x3, dest, src));
	break;
    }
    case CONV(DILL_F,DILL_U):
    case CONV(DILL_F,DILL_UL):
    case CONV(DILL_D,DILL_U):
    case CONV(DILL_D,DILL_UL):
    {
	int return_reg;
	if (from_type == DILL_F) {
	    x86_64_convert(c, DILL_F, DILL_D, XMM0, src);
	    src = XMM0;
	}
	return_reg = dill_scallul(c, (void*)dill_hidden_DtoUL, "%d", src);
	if (to_type == DILL_U) {
	    x86_64_lshi(c, dest, return_reg, 32);
	    x86_64_rshi(c, dest, dest, 32);
	} else {
	    x86_64_pmov(c, to_type, dest, return_reg);
	}
	break;
    }
    case CONV(DILL_F,DILL_I):
    case CONV(DILL_F,DILL_L):
    case CONV(DILL_D,DILL_I):
    case CONV(DILL_D,DILL_L):
    {
	int rex = 0;
	/* cvttsd2si */
	if (to_type == DILL_L) rex = REX_W;
	if (src > RDI) rex |= REX_B;
	if (dest > RDI) rex |= REX_R;
	BYTE_OUT1R3(c, (from_type == DILL_D) ? 0xf2 : 0xf3, rex, 0xf, 0x2c, ModRM(0x3, dest, src));
	break;
    }
    case CONV(DILL_I,DILL_D):
    case CONV(DILL_I,DILL_F):
    {
	int rex = 0;
	/* cvtsi2s{s,d} */
	if (src > RDI) rex |= REX_B;
	if (dest > RDI) rex |= REX_R;
	BYTE_OUT1R3(c, (to_type == DILL_D) ? 0xf2 : 0xf3, rex, 0xf, 0x2a, ModRM(0x3, dest, src));
	break;
    }
    case CONV(DILL_L,DILL_F):
    case CONV(DILL_L,DILL_D):
    case CONV(DILL_U,DILL_D):
    case CONV(DILL_U,DILL_F):
    {
	int rex = REX_W;
	/* cvtsi2s{s,d} */
	if (src > RDI) rex |= REX_B;
	if (dest > RDI) rex |= REX_R;
	BYTE_OUT1R3(c, (to_type == DILL_D) ? 0xf2 : 0xf3, rex, 0xf, 0x2a, ModRM(0x3, dest, src));
	break;
    }
    case CONV(DILL_UL,DILL_D):
    case CONV(DILL_UL,DILL_F):
    {
	int return_reg = dill_scalld(c, (void*)dill_hidden_ULtoD, "%l", src);
	if (to_type == DILL_F) {
	    BYTE_OUT1R3(c, 0xf2, REX_W, 0xf, 0x5a, ModRM(0x3, return_reg, return_reg));
	}
	x86_64_pmov(c, to_type, dest, return_reg);
	break;
    }
    case CONV(DILL_C,DILL_I):
    case CONV(DILL_C,DILL_L):
    case CONV(DILL_C,DILL_U):
    case CONV(DILL_C,DILL_UL):
	/* signext24 - lsh24, rsha24 */
	x86_64_lshi(c, dest, src, 56);
	x86_64_rshai(c, dest, dest, 56);
	break;
    case CONV(DILL_I, DILL_C):
    case CONV(DILL_U, DILL_C):
    case CONV(DILL_L, DILL_C):
    case CONV(DILL_UL, DILL_C):
	x86_64_andi(c, dest, src, 0xff);
	break;
    case CONV(DILL_S,DILL_I):
    case CONV(DILL_S,DILL_L):
    case CONV(DILL_S,DILL_U):
    case CONV(DILL_S,DILL_UL):
	/* signext48 - lsh48, rsha48 */
	x86_64_lshi(c, dest, src, 48);
	x86_64_rshai(c, dest, dest, 48);
	break;
    case CONV(DILL_US,DILL_I):
    case CONV(DILL_US,DILL_L):
    case CONV(DILL_US,DILL_U):
    case CONV(DILL_US,DILL_UL):
    case CONV(DILL_I, DILL_S):
    case CONV(DILL_U, DILL_S):
    case CONV(DILL_L, DILL_S):
    case CONV(DILL_UL, DILL_S):
    case CONV(DILL_I, DILL_US):
    case CONV(DILL_U, DILL_US):
    case CONV(DILL_L, DILL_US):
    case CONV(DILL_UL, DILL_US):
	/* zero uppper 48 - lsh48, rsh48 */
	x86_64_lshi(c, dest, src, 48);
	x86_64_rshi(c, dest, dest, 48);
	break;
    default:
	printf("Unknown case in x86_64 convert %d\n", CONV(from_type,to_type));
    }
}

static unsigned char op_conds[] = {
    0x84, /* dill_beq_code */  /* signed */
    0x8d, /* dill_bge_code */
    0x8F, /* dill_bgt_code */
    0x8e, /* dill_ble_code */
    0x8c, /* dill_blt_code */
    0x85, /* dill_bne_code */

    0x84, /* dill_beq_code */  /* unsigned */
    0x83, /* dill_bge_code */
    0x87, /* dill_bgt_code */ 
    0x86, /* dill_ble_code */
    0x82, /* dill_blt_code */
    0x85, /* dill_bne_code */
};

static unsigned char fop_conds[] = {
    0x84, /* dill_beq_code */   /* z = 1*/
    0x86, /* dill_bge_code */   /* jna */
    0x82, /* dill_bgt_code */
    0x83, /* dill_ble_code */   /* c = 0 */
    0x87, /* dill_blt_code */   
    0x85, /* dill_bne_code */
};

extern void
x86_64_branch(dill_stream c, int op, int type, int src1, int src2, int label)
{
    int rex = 0;
    if ((type == DILL_L) || (type == DILL_UL) || (type == DILL_P)) rex |= REX_W;

    switch(type) {
    case DILL_U:
    case DILL_UL:
	op += 6; /* second set of codes */
	/* fall through */
    default:
	if (src1 > RDI) rex |= REX_B;
	if (src2 > RDI) rex |= REX_R;
	if (type == DILL_D) {
	    BYTE_OUT1R3(c, 0x66, rex, 0x0f, 0x2e, ModRM(0x3, src2, src1));
	} else if (type == DILL_F) {
	    BYTE_OUT3R(c, rex, 0x0f, 0x2e, ModRM(0x3, src2, src1));
	} else {
	    BYTE_OUT2R(c, rex, 0x39, ModRM(0x3, src2, src1));
	}
	dill_mark_branch_location(c, label);
	if ((type == DILL_D) || (type == DILL_F)) {
	    BYTE_OUT2I(c, 0x0f, fop_conds[op], 0);
	} else {
	    BYTE_OUT2I(c, 0x0f, op_conds[op], 0);
	}
    }
    x86_64_nop(c);
}

extern void 
x86_64_jump_to_label(dill_stream c, unsigned long label)
{
    dill_mark_branch_location(c, label);
    BYTE_OUT1I(c, 0xe9, 0);
}

extern void x86_64_jump_to_reg(dill_stream c, unsigned long reg)
{
    int rex = 0;
    if (reg > RDI) rex |= REX_B;
    BYTE_OUT2R(c, rex, 0xff, ModRM(0x3, 0x4, reg));
}

extern void x86_64_jump_to_imm(dill_stream c, unsigned long imm)
{
    x86_64_seti(c, EAX, imm);
    BYTE_OUT2(c, 0xff, ModRM(0x3, 0x4, EAX));
}

extern void 
x86_64_jal(dill_stream c, int return_addill_reg, int target)
{
/* jump, source addr to return_addill_reg */
}

extern void 
x86_64_special(dill_stream c, special_operations type, long param)
{
    x86_64_mach_info smi = (x86_64_mach_info) c->p->mach_info;

    switch (type) {
    case DILL_NOP:
	break;
    case DILL_SEGMENTED_FOLLOWS:
	switch(param) {
	case DILL_X86_CS_PREFIX:
	    smi->pending_prefix = 0x2e;
	    break;
	case DILL_X86_SS_PREFIX:
	    smi->pending_prefix = 0x36;
	    break;
	case DILL_X86_DS_PREFIX:
	    smi->pending_prefix = 0x3e;
	    break;
	case DILL_X86_ES_PREFIX:
	    smi->pending_prefix = 0x26;
	    break;
	case DILL_X86_FS_PREFIX:
	    smi->pending_prefix = 0x64;
	    break;
	case DILL_X86_GS_PREFIX:
	    smi->pending_prefix = 0x65;
	    break;
	default:
	    fprintf(stderr, "Unknown x86 segment prefix!\n");
	}
	break;
    }
}

static void internal_push(dill_stream c, int type, int immediate, 
			  void *value_ptr)
{
    x86_64_mach_info smi = (x86_64_mach_info) c->p->mach_info;
    struct arg_info arg;

    arg.is_immediate = immediate;
    switch(type) {
    case DILL_C: case DILL_S:  case DILL_I: case DILL_L: case DILL_P: case DILL_EC:
	arg.type = DILL_L;
	break;
    case DILL_UC: case DILL_US: case DILL_U: case DILL_UL:
	arg.type = DILL_UL;
	break;
    case DILL_D: case DILL_F:
	arg.type = type;
        break;
    default:
	assert(0);
    }
	
    if ((arg.type != DILL_D) && (arg.type != DILL_F)) {
      if (smi->int_arg_count < 6) {
	arg.is_register = 1;
	arg.in_reg = arg.out_reg = arg_regs[smi->int_arg_count];
	smi->int_arg_count++;
      } else {
	arg.is_register = 0;
	arg.offset = smi->cur_arg_offset;
	smi->cur_arg_offset += 
	  roundup(type_info[(int)arg.type].size, smi->stack_align);
      }
    } else {
      if (smi->float_arg_count < 8) {
	arg.is_register = 1;
	arg.in_reg = arg.out_reg = smi->float_arg_count;
	smi->float_arg_count++;
      } else {
	arg.is_register = 0;
	arg.offset = smi->cur_arg_offset;
	smi->cur_arg_offset += 
	  roundup(type_info[(int)arg.type].size, smi->stack_align);
      }
    }
    if (arg.is_register == 0) {
	if (arg.offset == 0) {
	  smi->call_backpatch_offset = (char*)c->p->cur_ip - (char*)c->p->code_base;
	  dill_subli(c, ESP, ESP, 0x70909090);   /* tentative for backpatch */
	  smi->call_stack_space = 128;
	}
	/* store it on the stack only */
	if (arg.is_immediate) {
	    int arg_type = arg.type;
	    if (type == DILL_F) {
		float f = (float) *(double*)value_ptr;
		x86_64_setl(c, EAX, *(int*)&f);
		arg_type = DILL_I;
	    } else {
		x86_64_setl(c, EAX, *(long*)value_ptr);
		arg_type = DILL_L;
	    }
	    x86_64_pstorei(c, arg_type, 0, EAX, ESP, arg.offset);
	} else {
	    x86_64_pstorei(c, arg.type, 0, *(int*)value_ptr, ESP, 
			   arg.offset);
	}
    } else {
	if ((type != DILL_F) && (type != DILL_D)) {
	    if (arg.is_immediate) {
		x86_64_setl(c, arg.out_reg, *(long*)value_ptr);
	    } else {
		x86_64_pmov(c, type, arg.out_reg, *(int*) value_ptr);
	    }
	} else {
	    if (arg.is_immediate) {
		if ((type == DILL_F) || (type == DILL_D)) {
		    /* set appropriate register */
		    x86_64_setf(c, type, 0, arg.out_reg, 
			       *(double*)value_ptr);
		} else {
		    x86_64_setl(c, arg.out_reg, *(long*)value_ptr);
		}
	    } else {
		/* move to the appropriate float reg */
		x86_64_mov(c, type, 0, arg.out_reg, *(int*)value_ptr);
	    }

	}
    }
}

static void push_init(dill_stream c)
{
    x86_64_mach_info smi = (x86_64_mach_info) c->p->mach_info;
    smi->cur_arg_offset = 0;
    smi->int_arg_count = 0;
    smi->float_arg_count = 0;
    smi->float_arg_offset = 0;
    smi->call_stack_space = 0;
}

extern void x86_64_push(dill_stream c, int type, int reg)
{
    if ((type == DILL_V) && (reg == -1)) {
	push_init(c);
    } else {
	internal_push(c, type, 0, &reg);
    }
}

extern void x86_64_pushi(dill_stream c, int type, long value)
{
    internal_push(c, type, 1, &value);
}

extern void x86_64_pushpi(dill_stream c, int type, void *value)
{
    internal_push(c, type, 1, &value);
}

extern void x86_64_pushfi(dill_stream c, int type, double value)
{
    internal_push(c, type, 1, &value);
}

extern int x86_64_calli(dill_stream c, int type, void *xfer_address)
{
    int rex = REX_W;
    int tmp_call_reg = R11;
    if (tmp_call_reg > RDI) rex |= REX_B;
    
    /* save temporary registers */
    dill_mark_call_location(c, NULL, xfer_address);
    BYTE_OUT1LR(c, rex, 0xb8 + (0x7 & tmp_call_reg), 0);		/* setl */
    return x86_64_callr(c, type, R11);
}

extern int x86_64_callr(dill_stream c, int type, int src)
{
    x86_64_mach_info smi = (x86_64_mach_info) c->p->mach_info;
    int caller_side_ret_reg = EAX;
    int rex = REX_W;
    if (src > RDI) rex |= REX_B;

    /* save temporary registers */
    /* call through reg */
    x86_64_setl(c, EAX, smi->float_arg_count);
    BYTE_OUT2R(c, rex, 0xff, ModRM(0x3, 0x2, src));
    /* restore temporary registers */
    if ((type == DILL_D) || (type == DILL_F)) {
/*	caller_side_ret_reg = _f0;*/
    }
    if (smi->call_stack_space != 0) {
	void *save_ip = c->p->cur_ip;
	int call_stack_size = roundup(smi->call_stack_space, 8) + 16;

	/* backpatch space reservation */
	c->p->cur_ip = (char*)c->p->code_base + smi->call_backpatch_offset;
	dill_subli(c, ESP, ESP, call_stack_size);

	c->p->cur_ip = save_ip;
	/* undo arg space reservation */
	dill_addli(c, ESP, ESP, call_stack_size);
    }
    return caller_side_ret_reg;
}

extern void
x86_64_branchi(dill_stream c, int op, int type, int src, long imm, int label)
{
    int rex = 0;
    if ((type == DILL_L) || (type == DILL_UL) || (type == DILL_P)) rex |= REX_W;

    switch(type) {
    case DILL_F:
    case DILL_D:
	fprintf(stderr, "Shouldn't happen\n");
	break;
    case DILL_U:
    case DILL_UL:
/*
	switch(op) {
	case dill_bge_code: {
	    imm = imm-1;
	    op = dill_bgt_code;
	    break;
	}
	case dill_blt_code: {
	    imm = imm-1;
	    op = dill_ble_code;
	    break;
	}
	}
*/
	op += 6; /* second set of codes */
	/* fall through */
    default:
/*	BYTE_OUT2(c, 0x39, ModRM(0x3, src2, src1));*/
	if (src > RDI) rex |= REX_B;
	if (imm < 0xffffffff) {
	    BYTE_OUT2IR(c, rex, 0x81, ModRM(0x3, 0x7, src), imm);  /* cmp */
	} else {
	    x86_64_setl(c, EAX, imm);
	    BYTE_OUT2R(c, rex, 0x39, ModRM(0x3, EAX, src));
	}
	dill_mark_branch_location(c, label);
	BYTE_OUT2I(c, 0x0f, op_conds[op], 0);
    }
}

extern void x86_64_ret(dill_stream c, int data1, int data2, int src)
{
    switch (data1) {
    case DILL_C:
    case DILL_UC:
    case DILL_S:
    case DILL_US:
    case DILL_I:
    case DILL_U:
    case DILL_L:
    case DILL_UL:
    case DILL_P:
	if (src != EAX) x86_64_movl(c, EAX, src);
	break;
    case DILL_F:
	if (src != XMM0) x86_64_movf(c, XMM0, src);
	break;
    case DILL_D:
	if (src != XMM0) x86_64_movd(c, XMM0, src);
	break;
    }
    x86_64_simple_ret(c);
}

extern void x86_64_retf(dill_stream c, int data1, int data2, double imm)
{
    switch(data1) {
    case DILL_F:
	x86_64_setf(c, DILL_F, 0, XMM0, imm);
	break;
    case DILL_D:
	x86_64_setf(c, DILL_D, 0, XMM0, imm);
	break;
    }
}

extern void x86_64_reti(dill_stream c, int data1, int data2, long imm)
{
    switch (data1) {
    case DILL_C:
    case DILL_UC:
    case DILL_S:
    case DILL_US:
    case DILL_I:
    case DILL_U:
	x86_64_seti(c, EAX, imm);
	break;
    case DILL_L:
    case DILL_UL:
    case DILL_P:
	x86_64_setl(c, EAX, imm);
	break;
    }
    x86_64_simple_ret(c);
}

static void
x86_64_data_link(dill_stream c)
{
    struct branch_table *t = &c->p->branch_table;
    int i;
    for (i=0; i < t->data_mark_count; i++) {
	int label = t->data_marks[i].label;
	void *label_addr = t->label_locs[label] + (char*)c->p->code_base;
	*t->data_marks[i].addr = label_addr;
    }
}

static void
x86_64_branch_link(dill_stream c)
{
    struct branch_table *t = &c->p->branch_table;
    int i;

    for(i=0; i< t->branch_count; i++) {
	int label = t->branch_locs[i].label;
	int label_offset = t->label_locs[label] - t->branch_locs[i].loc;
	char *branch_addr = (char*)((char *)c->p->code_base + 
				  t->branch_locs[i].loc);
	int offset;
	if (*branch_addr == 0x0f) {
	    branch_addr +=2; /* conditional */
	    offset = 6;
	} else {
	    branch_addr++; /* unconditional */
	    offset = 5;
	}
	*(int*)branch_addr = label_offset - offset;
    }
}

static void
x86_64_call_link(dill_stream c)
{
    call_t *t = &c->p->call_table;
    int i;

    for(i=0; i< t->call_count; i++) {
	long *call_addr = (long *) ((unsigned long)c->p->code_base + 
				    t->call_locs[i].loc + 2);
	*call_addr = (unsigned long)t->call_locs[i].xfer_addr;
    }
}

static void
x86_64_emit_save(dill_stream c)
{
    x86_64_mach_info smi = (x86_64_mach_info) c->p->mach_info;
    void *save_ip = c->p->cur_ip;
    int ar_size = smi->act_rec_size;
    ar_size = roundup(ar_size, 16) + 8;

    c->p->cur_ip = (char*)c->p->code_base + smi->backpatch_offset;

    /* do local space reservation */
    dill_subli(c, ESP, ESP, ar_size);

    c->p->fp = (char*)c->p->code_base;
    c->p->cur_ip = save_ip;
}
    
static void
x86_64_flush(void *base, void *limit)
{
#if defined(HOST_X86_64)
    {
	volatile void *ptr = base;

	/* flush every 8 bytes of preallocated insn stream. */
	while((char*)ptr < (char*) limit) {
	    asm volatile ("clflush (%0)" : /* */ : "r" (ptr));
	    ptr = (char *)ptr + 8;
	}
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
    }
#endif
}    
extern void
x86_64_end(c)
dill_stream c;
{
    x86_64_simple_ret(c);
    x86_64_branch_link(c);
    x86_64_call_link(c);
    x86_64_data_link(c);
    x86_64_emit_save(c);
    x86_64_flush(c->p->code_base, c->p->code_limit);
}

extern void *
x86_64_clone_code(c, new_base, available_size)
dill_stream c;
void *new_base;
int available_size;
{
    int size = dill_code_size(c);
    void *old_base = c->p->code_base;
    void *native_base = c->p->code_base;
    if (available_size < size) {
	return NULL;
    }
    if (native_base == NULL) native_base = c->p->native.code_base;
    memcpy(new_base, native_base, size);
    c->p->code_base = new_base;
    c->p->cur_ip = new_base + size;
    c->p->fp = new_base;
    x86_64_branch_link(c);
    x86_64_call_link(c);
    x86_64_data_link(c);
    c->p->code_base = old_base;
    c->p->cur_ip = old_base + size;
    c->p->fp = old_base;
    return new_base;
}

extern void
x86_64_pset(dill_stream c, int type, int junk, int dest, long imm)
{
    switch(type) {
    case DILL_L: case DILL_UL: case DILL_P:
	x86_64_setl(c, dest, imm);
	break;
    default:
	x86_64_seti(c, dest, imm);
	break;
    }
}	

extern void
x86_64_setf(dill_stream c, int type, int junk, int dest, double imm)
{
    union {
	float f;
	int i;
    } a;
    union {
	double d;
	int i[2];
	long l;
    } b;
    if (type == DILL_F) {
	int rex = 0;
	a.f = imm;
	x86_64_seti(c, EAX, a.i);
	if (dest > RDI) rex |= REX_R;
	BYTE_OUT1R3(c, 0x66, rex, 0x0f, 0x6e, ModRM(0x3, dest, EAX));
    } else {
	int rex = REX_W;
	b.d = imm;
	x86_64_setl(c, EAX, b.l);
	if (dest > RDI) rex |= REX_R;
	BYTE_OUT1R3(c, 0x66, rex, 0x0f, 0x6e, ModRM(0x3, dest, EAX));
    }
}	


#define bit_R(x) (1<<x)

extern void
x86_64_reg_init(dill_stream c)
{
    c->p->var_i.init_avail[0] = (bit_R(EBX)|bit_R(R12)|bit_R(R13)|bit_R(R14)|bit_R(R15));
    c->p->var_i.members[0] = c->p->var_i.init_avail[0];
    c->p->tmp_i.init_avail[0] = (bit_R(R11));
    c->p->tmp_i.members[0] = c->p->tmp_i.init_avail[0] | bit_R(EAX);
    c->p->var_f.init_avail[0] = 0;
    c->p->var_f.members[0] = c->p->var_f.init_avail[0];
    c->p->tmp_f.init_avail[0] = (bit_R(XMM8)|bit_R(XMM9)|bit_R(XMM10)|bit_R(XMM11)|bit_R(XMM12)|bit_R(XMM13)|bit_R(XMM14)|bit_R(XMM15));
    c->p->tmp_f.members[0] = c->p->tmp_f.init_avail[0]|bit_R(XMM0)|bit_R(XMM1)|bit_R(XMM2)|bit_R(XMM3)|bit_R(XMM4)|bit_R(XMM5)|bit_R(XMM6)|bit_R(XMM7);
}

extern void*
gen_x86_64_mach_info(c)
dill_stream c;
{
    x86_64_mach_info smi = malloc(sizeof(*smi));
    if (c->p->mach_info != NULL) {
	free(c->p->mach_info);
	c->p->mach_info = NULL;
	c->p->native.mach_info = NULL;
    }
    x86_64_reg_init(c);
    smi->act_rec_size = 0;
    smi->stack_align = 8; /* 8 for x86_64 */
    smi->stack_constant_offset = 0; /* 2047 for x86_64v9 */
    smi->conversion_word = 0;
    smi->fcu_word = 0;
    smi->save_base = 0;
    smi->backpatch_offset = 0;
    return smi;
}
#if defined(HAVE_DIS_ASM_H) && !defined(NO_DISASSEMBLER)
/* GENERIC BINUTILS DISASSEMBLER */
#include "dis-asm.h"

#define MAXLENGTH (1<<23) /* Max length of function that can be disassembled */

#ifdef LINUX_KERNEL_MODULE
extern int
kfprintf(FILE *file, const char *fmt, ...)
{
    static char printk_buf[1024];
    int val;
    va_list ap;
    va_start(ap, fmt);
    val = vsnprintf(printk_buf, sizeof(printk_buf), fmt, ap);
    printk("%s", printk_buf);
    va_end(ap);
    return val;
}

#undef  stdout
#define stdout (FILE*)0
#define FPRINTF_FUNCTION kfprintf
#else
#define FPRINTF_FUNCTION fprintf
#endif
extern int
x86_64_init_disassembly_info(dill_stream c, void * ptr)
{
    struct disassemble_info *i = ptr;
#ifdef INIT_DISASSEMBLE_INFO_THREE_ARG
    INIT_DISASSEMBLE_INFO(*i, stdout, FPRINTF_FUNCTION);
    i->endian = BFD_ENDIAN_LITTLE;
#else
    INIT_DISASSEMBLE_INFO(*i, stdout);
#endif
    i->mach = bfd_mach_i386_i386;
    i->disassembler_options = "x86-64";
    if (c->p->code_base != NULL) {
	i->buffer = (bfd_byte *)c->p->code_base;
	i->buffer_vma = (bfd_vma)c->p->code_base;
    } else {
	i->buffer = (bfd_byte *)c->p->native.code_base;
	i->buffer_vma = (bfd_vma)c->p->native.code_base;
    }
    i->buffer_length = MAXLENGTH;
#ifdef HAVE_PRINT_INSN_I386
    return 1;
#else
    return 0;
#endif
}

extern int
x86_64_print_insn(dill_stream c, void *info_ptr, void *insn)
{
#ifdef HAVE_PRINT_INSN_I386
    return print_insn_i386((bfd_vma)insn, info_ptr);
#else
    return 0;
#endif
}

extern void null_func(){}
extern int
x86_64_count_insn(dill_stream c, int start, int end)
{
#ifdef HAVE_PRINT_INSN_I386
    struct disassemble_info i;
    int count;
    char *insn_ptr;
#ifdef INIT_DISASSEMBLE_INFO_THREE_ARG
    INIT_DISASSEMBLE_INFO(i, stdout, (fprintf_ftype) null_func);
    i.endian = BFD_ENDIAN_LITTLE;
#else
    INIT_DISASSEMBLE_INFO(i, stdout);
#endif
    i.mach = bfd_mach_i386_i386;
    if (c->p->code_base != NULL) {
	i.buffer = (bfd_byte *)c->p->code_base;
	i.buffer_vma = (bfd_vma)c->p->code_base;
    } else {
	i.buffer = (bfd_byte *)c->p->native.code_base;
	i.buffer_vma = (bfd_vma)c->p->native.code_base;
    }
    i.buffer_length = MAXLENGTH;
    count = 0;
    insn_ptr = i.buffer + start;
    while((long)insn_ptr < (long)i.buffer + end) {
	insn_ptr += print_insn_i386((bfd_vma)insn_ptr, &i);
	count++;
    }
    return count;
#else
   /* no print insn, just return the buffer length */
    return end - start;
#endif
}
#else
extern int
x86_64_count_insn(dill_stream c, int start, int end)
{   /* no print insn, just return the buffer length */
    return end - start;
}
extern int
x86_64_init_disassembly_info(dill_stream c, void * ptr){return 0;}
extern int x86_64_print_insn(dill_stream c, void *info_ptr, void *insn){return 0;}
#endif

char *char_regs[] = {"AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH"};
char *short_regs[] = {"AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI"};
char *int_regs[] = {"EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI", "EDI"};
char *long_regs[] = {"RAX", "RCX", "RDX", "RBX", "RSP", "RBP", "RSI", "RDI"};
char *float_regs[] = {"RAX", "RCX", "RDX", "RBX", "RSP", "RBP", "RSI", "RDI"};

extern void
x86_64_print_reg(dill_stream c, int typ, int reg)
{
    switch(typ) {
    case DILL_C: case DILL_UC:
	if ((reg > 0) && (reg < sizeof(char_regs)/sizeof(char_regs[0]))) {
	    printf("%s", char_regs[reg]);
	    return;
	}
	break;
    case DILL_S: case DILL_US:
	if ((reg > 0) && (reg < sizeof(short_regs)/sizeof(short_regs[0]))) {
	    printf("%s", short_regs[reg]);
	    return;
	}
	break;
/*    case DILL_C: case DILL_UC: case DILL_S: case DILL_US:*/
    case DILL_I: case DILL_U: case DILL_L: case DILL_UL:
	if ((reg > 0) && (reg < sizeof(int_regs)/sizeof(int_regs[0]))) {
	    printf("%s", int_regs[reg]);
	    return;
	}
	break;
    case DILL_F: case DILL_D:
	printf("Fstack");
	return;
    }
    printf("NoReg(%d)", reg);
}
	
