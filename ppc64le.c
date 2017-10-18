#include "dill.h"
#include "dill_internal.h"
#include "ppc64le.h"
#include "config.h"
#include <stdio.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#undef NDEBUG
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define lo16(im) (((long)im) & 0xffff)
#define hi16(im) ((((long)im) & 0xffffffff) >> 16)

#define ppc64le_savei(s, imm) 
#define ppc64le_ori(s, dest, src, imm) 	INSN_OUT(s, D_FORM(24, dest, src, lo16(imm)));
#define ppc64le_andi(s, dest, src, imm) INSN_OUT(s, D_FORM(28, dest, src, lo16(imm)));
#define ppc64le_or(s, dest, src1, src2) INSN_OUT(s, X_FORM(31,src1, dest, src2, 444))
#define ppc64le_movl(s, dest, src) ppc64le_or(s, dest, src, src);
#define ppc64le_int_mov(s, dest, src) ppc64le_or(s, dest, src, src)
#define ppc64le_movf(s, dest, src) INSN_OUT(s, X_FORM(63, dest, 0, src, 72))
#define ppc64le_movd(s, dest, src) INSN_OUT(s, X_FORM(63, dest, 0, src, 72))

#define ppc64le_lshi(s, dest, src,imm) INSN_OUT(s, MD_FORM(30,dest,src,imm & 0x1f,63-imm, 0, imm>>5));
#define ppc64le_xlshi(s, dest, src1,imm) INSN_OUT(s, HDR(0x2)|OP(0x25)|RD(dest)|RS1(src1)|IM|SIMM13(imm)|(1<<12) );
#define ppc64le_rshi(s, dest, src1,imm) INSN_OUT(s, HDR(0x2)|OP(0x26)|RD(dest)|RS1(src1)|IM|SIMM13(imm));
#define ppc64le_xrshi(s, dest, src1,imm) INSN_OUT(s, HDR(0x2)|OP(0x26)|RD(dest)|RS1(src1)|IM|SIMM13(imm)|(1<<12) );
#define ppc64le_rshai(s, dest, src1,imm) INSN_OUT(s, HDR(0x2)|OP(0x27)|RD(dest)|RS1(src1)|IM|SIMM13(imm));
#define ppc64le_xrshai(s, dest, src1,imm) INSN_OUT(s, HDR(0x2)|OP(0x27)|RD(dest)|RS1(src1)|IM|SIMM13(imm)|(1<<12) );
#define ppc64le_rsh(s, dest, src1, src2) 	INSN_OUT(s, HDR(0x2)|OP(0x27)|RD(dest)|RS1(src1)|RS2(src2));

#define ppc64le_nop(c) INSN_OUT(s, 0x60000000);

#define IREG 0
#define FREG 1

#define roundup(a,b) ((a + (b-1)) & (-b))

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

int ppc64le_type_align[] = {
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
	1, /* V */
        8, /* B */
	sizeof(long), /* EC */
};

int ppc64le_type_size[] = {
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
	1, /* V */
        8, /* B */
	sizeof(long), /* EC */
};

static void ppc64le_simple_ret(dill_stream s);
static void ppc64le_spill_fill_vars(dill_stream s, int action);

static void
dump_bits(int val) 
{
    int i;
    for (i = 0; i < 32; i++) {
	printf("%2d", (val & 0x80000000) >> 31);
	val = val << 1;
    }
    printf("\n");
}

extern int
ppc64le_local(dill_stream s, int type)
{
    ppc64le_mach_info smi = (ppc64le_mach_info) s->p->mach_info;

    smi->act_rec_size += roundup(type_info[type].size, smi->stack_align);
    return (-smi->act_rec_size) + smi->stack_constant_offset;
}

extern int
ppc64le_localb(dill_stream s, int size)
{
    ppc64le_mach_info smi = (ppc64le_mach_info) s->p->mach_info;
    if (size < 0) size = 0;
    smi->act_rec_size = roundup(smi->act_rec_size, size);

    smi->act_rec_size += roundup(size, smi->stack_align);
    return (-smi->act_rec_size) - 8 + smi->stack_constant_offset;
}

extern int ppc64le_local_op(dill_stream s, int flag, int val)
{
    int size = val;
    if (flag == 0) {
	size = type_info[val].size;
    }
    if (size < 0) size = 0;
    return ppc64le_localb(s, size);
}	

static int 
is_temp(int ireg)
{
  /*    return ((ireg <= _g7) && (ireg >= _g0));*/
}

extern void
ppc64le_save_restore_op(dill_stream s, int save_restore, int type, int reg)
{
    ppc64le_mach_info smi = (ppc64le_mach_info) s->p->mach_info;
    if (save_restore == 0) { /* save */
	switch (type) {
	case DILL_D: case DILL_F:
	    ppc64le_pstorei(s, type, 0, reg, _sp, smi->fp_save_offset + reg * smi->stack_align + smi->stack_constant_offset);
	    break;
	default:
	    if (is_temp(reg)) {
	      /*		ppc64le_pstorei(s, type, 0, reg, _sp, smi->gp_save_offset + (reg - _g0) * smi->stack_align + smi->stack_constant_offset);*/
	    }
	    break;
	}
    } else {  /* restore */
	switch (type) {
	case DILL_D: case DILL_F:
	    ppc64le_ploadi(s, type, 0, reg, _sp, smi->fp_save_offset + reg * smi->stack_align + smi->stack_constant_offset);
	    break;
	default:
	    if (is_temp(reg)) {
		ppc64le_ploadi(s, type, 0, reg, _sp, smi->gp_save_offset + (reg - _gpr0) * smi->stack_align + smi->stack_constant_offset);
	    }
	    break;
	}
    }
}	

static void
ppc64le_movi2d(dill_stream s, int dest, int src);

static void
ppc64le_movi2f(dill_stream s, int dest, int src)
{
    /* stwu  src, -16(r1) */
    INSN_OUT(s, D_FORM(37, src, _gpr1, -16 & 0xffff));
    /* lfs dest, (r1) */
    INSN_OUT(s, D_FORM(49, dest, _gpr1, 0));
    /* addi r1, r1, 16 */
    INSN_OUT(s, D_FORM(14, _gpr1, _gpr1, 16));
}

static void
ppc64le_movf2i(dill_stream s, int dest, int src)
{
  /*    ppc64le_mach_info smi = (ppc64le_mach_info) s->p->mach_info;
    ppc64le_pstorei(s, DILL_F, 0, src, _fp, smi->conversion_word);
	  ppc64le_ploadi(s, DILL_I, 0, dest, _fp, smi->conversion_word);*/
}
    
static void
ppc64le_movd2i(dill_stream s, int dest, int src)
{
  /*    ppc64le_mach_info smi = (ppc64le_mach_info) s->p->mach_info;
        ppc64le_pstorei(s, DILL_D, 0, src, _fp, smi->conversion_word);
    if (smi->stack_align == 8) {
	ppc64le_ploadi(s, DILL_L, 0, dest, _fp, smi->conversion_word);
    } else {
	ppc64le_ploadi(s, DILL_I, 0, dest, _fp, smi->conversion_word);
	ppc64le_ploadi(s, DILL_I, 0, dest+1, _fp, smi->conversion_word+4);
	}*/
}
    
static void
ppc64le_movi2d(dill_stream s, int dest, int src)
{
    /* stdu  src, -16(r1) */
    INSN_OUT(s, D_FORM(62, src, _gpr1, (-16 & 0xfffc) | 0x1));
    /* lfd dest, (r1) */
    INSN_OUT(s, D_FORM(50, dest, _gpr1, 0));
    /* addi r1, r1, 16 */
    INSN_OUT(s, D_FORM(14, _gpr1, _gpr1, 16));
}
    
extern void ppc64le_farith(s, op, xop, dest, src1, src2)
dill_stream s;
int op;
int xop;
int dest;
int src1;
int src2;
{
    INSN_OUT(s, XX3_FORM(op, dest, src1, src2, xop));
}

extern void ppc64le_XOFORM_arith(s, ppc64le_po, ppc64le_xo, dest, src1, src2)
dill_stream s;
int ppc64le_po;
int ppc64le_xo;
int dest;
int src1;
int src2;
{
    INSN_OUT(s, XO_FORM(ppc64le_po, dest, src1, src2, ppc64le_xo));
}

extern void ppc64le_swap_arith(s, ppc64le_po, ppc64le_xo, dest, src1, src2)
dill_stream s;
int ppc64le_po;
int ppc64le_xo;
int dest;
int src1;
int src2;
{
    INSN_OUT(s, XO_FORM(ppc64le_po, dest, src2, src1, ppc64le_xo));
}

extern void ppc64le_shift_arith(s, ppc64le_po, type, dest, src1, src2)
dill_stream s;
int ppc64le_po;
int type;
int dest;
int src1;
int src2;
{
    INSN_OUT(s, XO_FORM(31, src1, dest, src2, ppc64le_po));
    if (type == DILL_I) {
	/* clrldi dest,dest,32 */
	INSN_OUT(s, MD_FORM(30,dest,dest,0,32, 0, 0));
    }
}

extern void ppc64le_log_arith(s, ppc64le_po, ppc64le_xo, dest, src1, src2)
dill_stream s;
int ppc64le_po;
int ppc64le_xo;
int dest;
int src1;
int src2;
{
    INSN_OUT(s, XO_FORM(ppc64le_po, src1, dest, src2, ppc64le_xo));
}

extern void ppc64le_imm_arith(s, op, full_op, dest, src1, imm)
dill_stream s;
int op;
int full_op;
int dest;
int src1;
long imm;
{
    if (((long)imm) < 32767 && ((long)imm) >= -32768) {
	/* D-FORM */
	INSN_OUT(s, D_FORM(op, dest, src1, imm));
    } else {
	ppc64le_set(s, dest, imm);
	if (full_op == 40) {  /* special SUB case */
	    ppc64le_XOFORM_arith(s, 31, full_op, dest, dest, src1);
	} else {
	    ppc64le_XOFORM_arith(s, 31, full_op, dest, src1, dest);
	}
    }
}

extern void ppc64le_shiftimm_arith(s, op, type, dest, src1, imm)
dill_stream s;
int op;
int type;
int dest;
int src1;
long imm;
{
    
    if (op == 413) {
	/* rsh, different form */
	if ((type == DILL_I) || (type == DILL_L)) {
	    INSN_OUT(s, XS_FORM(31, src1, dest, imm & 0x1f, 413, imm >> 5));
	} else {
	    INSN_OUT(s, MD_FORM(30, src1, dest, (64-imm)&0x1f, (((imm&0x1f)<< 1) | (imm >> 5)), 0, ((64-imm)>> 5)));
	}
    } else {
	/* lsh */
	if (dill_type_size(s, type) == 4) {
	    INSN_OUT(s, M_FORM(op, src1, dest, imm, 0, 31 - imm));
	} else {
	    INSN_OUT(s, MD_FORM(30, src1, dest, (imm&0x1f), ((((63-imm)&0x1f) << 1) | ((63-imm)>>5)), 1, (imm>>5)));
	}
    }
    if (dill_type_size(s, type) == 4) {
	/* clrldi dest,dest,32 */
	INSN_OUT(s, MD_FORM(30,dest,dest,0,32, 0, 0));
    }
}

extern void ppc64le_logimm_arith(s, op, full_op, dest, src1, imm)
dill_stream s;
int op;
int full_op;
int dest;
int src1;
long imm;
{
    if ((imm >> 16) == 0) {
	/* D-FORM */
	INSN_OUT(s, D_FORM(op, dest, src1, imm));
//    } if ((((unsigned long)imm) >> 32) == 0) {
//	INSN_OUT(s, D_FORM(op, dest, src1, (((unsigned long)imm) & 0xffff)));
//	/* shifted version is op+1 for logical ops */
//	INSN_OUT(s, D_FORM(op+1, dest, src1, ((((unsigned long)imm) >> 16) & 0xffff)));
    } else {
	ppc64le_set(s, dest, imm);
	ppc64le_XOFORM_arith(s, 31, full_op, src1, dest, dest);
    }
}

enum { SPILL = 1, FILL = 2 };

static int 
int_reg_store_offset(dill_stream s, int reg)
{
    ppc64le_mach_info smi = (ppc64le_mach_info) s->p->mach_info;

    int offset = 48 + 64 + smi->act_rec_size;
    int reg_offset = (reg - _gpr14) * 8;
    return offset + reg_offset;
}

static int 
float_reg_store_offset(dill_stream s, int reg)
{
    int offset = int_reg_store_offset(s, _gpr31) + 8;
    int reg_offset = (reg - _fpr14) * 8;
    return offset + reg_offset;
}

static int
stack_space_calc(dill_stream s)
{
// Reserve Space
// 48 (save areas) + 64 (parameter area) + (local variables) + (non-volatile save)
// align to 16-byte boundary
    ppc64le_mach_info smi = (ppc64le_mach_info) s->p->mach_info;

    int stack_space = 48 + 64 + smi->act_rec_size;
    /* integer save */
    stack_space += 18 /* non-volatile regs */ * 8;
    /* float save */
    stack_space += 18 /* non-volatile regs */ * 8;
    
    /* round up to multiple of 16 */
    stack_space = ((stack_space + 15) / 16) * 16;
    return stack_space;
}

static void
ppc64le_emit_proc_epilogue(dill_stream s)
{
    ppc64le_mach_info smi = (ppc64le_mach_info) s->p->mach_info;
    int stack_size = stack_space_calc(s);

    /* 
     *  all return operations will branch here.  
     * _gpr3 or _fpr1 may hold return data, we won't stomp on it. 
     */
    dill_mark_label(s, smi->epilogue_label);

    /* 
     * restore non-volatile variables that we might have used in 
     * the course of the subroutine.
     */
    ppc64le_spill_fill_vars(s, FILL);

    /*
     * destroy the activation record
     */

    /* ld r0, 64(r1) */
    ppc64le_ploadi(s, DILL_L, 0, _gpr0, _sp, 64);
    /* mtlr r0 */
    INSN_OUT(s, XFX_FORM(31, _gpr0, /* LR */ 0x100, 467));
    /* addi r1, r1, STACK_SIZE */
    INSN_OUT(s, D_FORM(14, _gpr1, _gpr1, stack_size));
    /* blr*/
    INSN_OUT(s, XL_FORM(19,0x14,0,0,16,0));
}

static void
ppc64le_emit_proc_prologue(dill_stream s)
{
    ppc64le_mach_info smi = (ppc64le_mach_info) s->p->mach_info;

    /* 
     * this is the real entry point, but we emit this after the rest of the
     * code so that we can save only the non-volatile regs that we've
     * had to use.  At the end of this prologue we branch to the beginning
     * of the actual subroutine code (emitted earlier).
     */
    int stack_size = stack_space_calc(s);

    /*
     * create the activation record
     */
    smi->save_insn_offset = (long)s->p->cur_ip - (long)s->p->code_base;


    /* addis r2, R12, 0 */
    INSN_OUT(s, D_FORM(15, _gpr2, _gpr12, 0));
    /* addi r2, R2, 0 */
    INSN_OUT(s, D_FORM(14, _gpr2, _gpr2, 0));
    

    /* mflr r0 */
    INSN_OUT(s, XFX_FORM(31, _gpr0, /* LR */ 0x100, 339));
    /* stdu  _gpr1, -SAVE_AREA(r1) */
    INSN_OUT(s, D_FORM(62, _gpr1, _gpr1, (-stack_size & 0xfffc) | 0x1));
    /* std  _lr, 64(r1) */
    INSN_OUT(s, D_FORM(62, _gpr0, _sp, (64 & 0xfffc)));

    /*
     *  Spill the non-volatiles that we'll destroy.
     */
    ppc64le_spill_fill_vars(s, SPILL);

    /*
     * go do the actual code
     */
    ppc64le_jump_to_label(s, smi->start_label);
    s->p->fp = (char*)s->p->code_base + smi->save_insn_offset;
    s->p->disassembly_code_start = s->p->code_base;
}

/*
  The PPC activation record looks like this:

High Address

          +-> Back chain
          |   Floating point register save area
          |   General register save area
          |   VRSAVE save word (32-bits)
          |   Alignment padding (4 or 12 bytes)
          |   Vector register save area (quadword aligned)
          |   Local variable space
          |   Parameter save area    (SP + 48)
          |   TOC save area          (SP + 40)
          |   link editor doubleword (SP + 32)
          |   compiler doubleword    (SP + 24)
          |   LR save area           (SP + 16)
          |   CR save area           (SP + 8)
SP  --->  +-- Back chain             (SP + 0)

Low Address

*/
extern void
ppc64le_proc_start(dill_stream s, char *subr_name, int arg_count, arg_info_list args,
	     dill_reg *arglist)
{
    int i;

    int max_in_reg = _gpr2;
    ppc64le_mach_info smi = (ppc64le_mach_info) s->p->mach_info;
    int cur_arg_offset = 32;
    int last_incoming_fp_arg = _fpr0;
    int last_incoming_int_arg = _gpr2;

    smi->start_label = dill_alloc_label(s, "Procedure code start");
    smi->epilogue_label = dill_alloc_label(s, "Procedure code exit");
    dill_mark_label(s, smi->start_label);

//    smi->conversion_word = ppc64le_local(s, DILL_D);
//    smi->conversion_word = ppc64le_local(s, DILL_D);

    /* load params from regs */
    for (i = 0; i < arg_count; i++) {
	switch (args[i].type) {
	case DILL_F: case DILL_D: {
	    int reg;
	    if (last_incoming_fp_arg < _fpr13) {
		args[i].is_register = 1;
		reg = ++last_incoming_fp_arg;
		args[i].in_reg = args[i].out_reg = reg;
	    } else {
		args[i].is_register = 0;
	    }
	    break;
	}
	default:
	    if (last_incoming_int_arg < _gpr10) {
		args[i].is_register = 1;
		args[i].in_reg = ++last_incoming_int_arg;
		args[i].out_reg = args[i].out_reg;
		max_in_reg = args[i].in_reg;
	    } else {
		args[i].is_register = 0;
	    }
	    break;
	}
	args[i].offset = cur_arg_offset;
	cur_arg_offset += roundup(type_info[(int)args[i].type].size, smi->stack_align);
    }
    
    for (i = 0; i < arg_count; i++) {
	int tmp_reg;
	if (!dill_raw_getreg(s, &tmp_reg, args[i].type, DILL_VAR)) {
	    fprintf(stderr, "not enough registers for parameter %d\n", i);
	    exit(1);
	}
	if (arglist != NULL) arglist[i] = tmp_reg;
	if (args[i].is_register) {
	    if ((args[i].type != DILL_F) && (args[i].type != DILL_D)) {
		ppc64le_int_mov(s, tmp_reg, args[i].in_reg);
	    } else {
		ppc64le_movf(s, tmp_reg, args[i].in_reg);
	    }
	} else {
	    /* load the old SP into our int temporary */
	    ppc64le_ploadi(s, DILL_P, 0, tmp_reg, _sp, 0);
	    /* use our int temporary to load the value */
	    ppc64le_ploadi(s, args[i].type, 0, tmp_reg, tmp_reg, args[i].offset);
	}
	args[i].in_reg = tmp_reg;
	args[i].is_register = 1;
    }
}

static short ldi_opcodes[] = {
    34, /* DILL_C */
    34, /* DILL_UC */
    42, /* DILL_S */
    40, /* DILL_US */
    58, /* DILL_I */
    32, /* DILL_U */
    58, /* DILL_L */
    58, /* DILL_UL */
    58, /* DILL_P */
    48,  /* DILL_F */
    50,  /* DILL_D */
    0x00, /* DILL_V */
    0x00, /* DILL_B */
    0x0b, /* DILL_EC */
};

extern void
ppc64le_ploadi(dill_stream s, int type, int junk, int dest, int src, long offset)
{
    if  (((long)offset) >= 32767 || ((long)offset) < -32768) {
	ppc64le_set(s, _gpr0, offset);
	ppc64le_pload(s, type, junk, dest, src, _gpr0);
	return;
    }

    switch (type) {
    case DILL_L: case DILL_UL: case DILL_P:{
	ppc64le_mach_info smi = (ppc64le_mach_info) s->p->mach_info;
	if (smi->stack_align == 4) {
	    type = DILL_I;
	}
    }
    /* fall through */
    default:
	INSN_OUT(s, D_FORM(ldi_opcodes[type], dest, src, offset));
	break;
    }
}

static short ld_opcodes[] = {
    87, /* DILL_C */
    87, /* DILL_UC */
    343, /* DILL_S */
    279, /* DILL_US */
    341, /* DILL_I */
    23, /* DILL_U */
    21, /* DILL_L */
    21, /* DILL_UL */
    21, /* DILL_P */
    535,  /* DILL_F */
    599,  /* DILL_D */
    0x00, /* DILL_V */
    0x00, /* DILL_B */
    0x0b, /* DILL_EC */
};

extern void
ppc64le_pload(dill_stream s, int type, int junk, int dest, int src1, int src2)
{
    switch (type) {
    case DILL_L: case DILL_UL: case DILL_P:
    {
	ppc64le_mach_info smi = (ppc64le_mach_info) s->p->mach_info;
	if (smi->stack_align == 4) {
	    type = DILL_I;
	}
	/* fall through */
    }
    default:
	INSN_OUT(s, X_FORM(31, dest, src1, src2, ld_opcodes[type]));
	break;
    }
}

static char ld_bs_opcodes[] = {  /* load from alternate space */
    0x19, /* DILL_C */
    0x11, /* DILL_UC */
    0x1a, /* DILL_S */
    0x12, /* DILL_US */
    0x18, /* DILL_I */
    0x10, /* DILL_U */
    0x1b, /* DILL_L */
    0x1b, /* DILL_UL */
    0x1b, /* DILL_P */
    0x30, /* DILL_F */
    0x33, /* DILL_D */
};
extern void
ppc64le_pbsloadi(dill_stream s, int type, int junk, int dest, int src, long offset)
{
    if (offset == 0) {
	ppc64le_pbsload(s, type, junk, dest, src, _gpr2);
    } else {
	ppc64le_set(s, _gpr2, offset);
	ppc64le_pbsload(s, type, junk, dest, src, _gpr2);
    }
}


extern void
ppc64le_pbsload(dill_stream s, int type, int junk, int dest, int src1, int src2)
{
    switch (type) {
    case DILL_L: case DILL_UL: case DILL_P:
    {
	ppc64le_mach_info smi = (ppc64le_mach_info) s->p->mach_info;
	if (smi->stack_align == 4) {
	    type = DILL_I;
	}
	/* fall through */
    }
    default:
	INSN_OUT(s, HDR(0x3)|RD(dest)|OP(ld_bs_opcodes[type])|RS1(src1)|RS2(src2)|ASI(0x88));
	break;
    }
}

static char sti_opcodes[] = {
    38, /* DILL_C */
    38, /* DILL_UC */
    44, /* DILL_S */
    44, /* DILL_US */
    36, /* DILL_I */
    36, /* DILL_U */
    62, /* DILL_L */
    62, /* DILL_UL */
    62, /* DILL_P */
    52,  /* DILL_F */
    54,  /* DILL_D */
    0x00, /* DILL_V */
    0x00, /* DILL_B */
    0x0e, /* DILL_EC */
};
extern void
ppc64le_pstorei(dill_stream s, int type, int junk, int dest, int src, long offset)
{
    if  (((long)offset) >= 32767 || ((long)offset) < -32768) {
	ppc64le_set(s, _gpr0, offset);
	ppc64le_pstore(s, type, junk, dest, src, _gpr0);
	return;
    }

    switch (type) {
    case DILL_L: case DILL_UL: case DILL_P:{
	ppc64le_mach_info smi = (ppc64le_mach_info) s->p->mach_info;
	if (smi->stack_align == 4) {
	    type = DILL_I;
	}
    }
    /* fall through */
    default:
	INSN_OUT(s, D_FORM(sti_opcodes[type],dest, src, offset));
	break;
    }
}

static short st_opcodes[] = {
    215, /* DILL_C */
    215, /* DILL_UC */
    407, /* DILL_S */
    407, /* DILL_US */
    151, /* DILL_I */
    151, /* DILL_U */
    149, /* DILL_L */
    149, /* DILL_UL */
    149, /* DILL_P */
    663,  /* DILL_F */
    727,  /* DILL_D */
    0x00, /* DILL_V */
    0x00, /* DILL_B */
    0x0e, /* DILL_EC */
};

extern void
ppc64le_pstore(dill_stream s, int type, int junk, int dest, int src1, int src2)
{
    switch (type) {
    case DILL_L: case DILL_UL: case DILL_P:
    {
	ppc64le_mach_info smi = (ppc64le_mach_info) s->p->mach_info;
	if (smi->stack_align == 4) {
	    type = DILL_I;
	}
	/* fall through */
    }
    default:
	INSN_OUT(s, X_FORM(31, dest, src1, src2, st_opcodes[type]));
	break;
    }
}

extern long dill_ppc64le_hidden_modi(int a, int b);
extern long dill_ppc64le_hidden_mod(long a, long b);
extern unsigned long dill_ppc64le_hidden_umod(unsigned long a, unsigned long b);
extern unsigned int dill_ppc64le_hidden_umodi(unsigned int a, unsigned int b);
extern double dill_ppc64le_hidden_ultod(unsigned long a);
extern float dill_ppc64le_hidden_ultof(unsigned long a);
extern unsigned long dill_ppc64le_hidden_dtoul(double a);
extern unsigned int dill_ppc64le_hidden_dtou(double a);
extern unsigned long dill_ppc64le_hidden_ftoul(float a);
extern unsigned int dill_ppc64le_hidden_ftou(float a);
extern long dill_ppc64le_hidden_udiv(unsigned long a, unsigned long b);


extern void ppc64le_mod(dill_stream s, int is_signed, int type_long, int dest, 
		      int src1, int src2)
{
    if (is_signed) {
	if (type_long) {
	    /* divd */
	    INSN_OUT(s, XO_FORM(31, _gpr0, src1, src2, 489));
	} else {
	    /* divw */
	    INSN_OUT(s, XO_FORM(31, _gpr0, src1, src2, 491));
	}	    
    } else {
	if (type_long) {
	    /* divdu */
	    INSN_OUT(s, XO_FORM(31, _gpr0, src1, src2, 457));
	} else {
	    /* divdu */
	    INSN_OUT(s, XO_FORM(31, _gpr0, src1, src2, 459));
	}
    }
    /* muld */
    INSN_OUT(s, XO_FORM(31, _gpr0, _gpr0, src2, 233));
    /* subf */
    INSN_OUT(s, XO_FORM(31, dest, _gpr0, src1, 40));
}

extern void ppc64le_modi(dill_stream s, int data1, int data2, int dest, int src1, 
		      long imm)
{
    ppc64le_set(s, dest, imm);
    ppc64le_mod(s, data1, data2, dest, src1, dest);
}

extern void ppc64le_div(dill_stream s, int op, int type_long, int dest, int src1,
		      int src2)
{
    ppc64le_mach_info smi = (ppc64le_mach_info) s->p->mach_info;
    if ((op == 0x0d /* udiv */) && (type_long == 1)) {
	int return_reg;
	return_reg = dill_scalll(s, (void*)&dill_ppc64le_hidden_udiv, "dill_ppc64le_hidden_udiv", "%l%l", src1, src2);
	dill_movl(s, dest, return_reg);
	return;

    }
    INSN_OUT(s, XO_FORM(31, dest, src1, src2, op));
}

extern void ppc64le_divi(dill_stream s, int op, int type_long, 
		       int dest, int src, long imm)
{
    ppc64le_mach_info smi = (ppc64le_mach_info) s->p->mach_info;
    ppc64le_set(s, dest, imm);
    ppc64le_div(s, op, type_long, dest, src, dest);
}

extern void
ppc64le_mov(dill_stream s, int type, int junk, int dest, int src)
{
    if (src == dest) return;
    switch(type) {
    case DILL_D:
	ppc64le_movd(s, dest, src);
	break;
    case DILL_F:
	ppc64le_movf(s, dest, src);
	break;
    default:
	ppc64le_movl(s, dest, src);
    }
}

extern void
ppc64le_lea(dill_stream s, int j1, int j2, int dest, int src, long imm)
{
#ifdef NOTDEF
    ppc64le_mach_info smi = (ppc64le_mach_info) s->p->mach_info;
    if (src != _fp) {
	/* ppc64le_add */
	ppc64le_FORM3imm_arith(s, 0, 0, dest, src, imm);
    } else {
	ppc64le_FORM3imm_arith(s, 0, 0, dest, src, 
			     imm  + smi->stack_constant_offset);
    }
#endif
}
	
static void
ppc64le_saverestore_floats(dill_stream s, int saverestore)
{
    int i;
    for (i=2; i <32 ; i+=2) {
	if (dill_mustsave(&s->p->tmp_f, i)) {
	    ppc64le_save_restore_op(s, saverestore, DILL_D, i);
	}
    }
}


extern void ppc64le_FORM2_arith(s, op3, op, dest, src)
dill_stream s;
int op3;
int op;
int dest;
int src;
{
    if (op3) {
	INSN_OUT(s, X_FORM(op3, src, dest, src, op));
    } else {
	/* must be not */
	/* cmpwi, mfcr, rlwinm 3, 31, 31 */
	INSN_OUT(s, D_FORM(11, 0<<2, src, 0));
	/* mfcr */
	INSN_OUT(s, (31<<26)| (dest<<21)|(19<<1)); 
	INSN_OUT(s, M_FORM(21, dest, dest, 3, 31, 31));
    }
}

#define CONV(x,y) ((x*100)+y)
extern void
ppc64le_convert(dill_stream s, int from_type, int to_type, 
	      int dest, int src)
{
    ppc64le_mach_info smi = (ppc64le_mach_info) s->p->mach_info;
    int word_size = smi->stack_align << 3;

    from_type &= 0xf;
    to_type &= 0xf;
    switch(CONV(from_type, to_type)) {
    case CONV(DILL_I,DILL_L):
	/* extsw */
	INSN_OUT(s, X_FORM(31, src, dest, 0, 986));
	break;
    case CONV(DILL_US,DILL_S):
    case CONV(DILL_UC,DILL_US):
    case CONV(DILL_C,DILL_US):
    case CONV(DILL_C,DILL_UC):
    case CONV(DILL_I,DILL_U):
    case CONV(DILL_I,DILL_UL):
    case CONV(DILL_UL,DILL_I):
    case CONV(DILL_UL,DILL_U):
    case CONV(DILL_L,DILL_U):
    case CONV(DILL_U,DILL_UL):
    case CONV(DILL_U,DILL_L):
    case CONV(DILL_L,DILL_I):
    case CONV(DILL_UL,DILL_L):
    case CONV(DILL_L,DILL_UL):
    case CONV(DILL_P,DILL_UL):
    case CONV(DILL_UL,DILL_P):
    case CONV(DILL_U,DILL_I):
	if(src == dest) return;
	ppc64le_movl(s, dest,src);
	break;
    case CONV(DILL_F,DILL_D):
	ppc64le_movd(s, dest, src);
	break;
    case CONV(DILL_F,DILL_L):
	INSN_OUT(s, XX2_FORM(60, src, src, 344));
	INSN_OUT(s, X_FORM(31, src, dest, 0, 51));
	break;
    case CONV(DILL_F,DILL_I):
	INSN_OUT(s, X_FORM(31, src, dest, 0, 51));
	break;
    case CONV(DILL_F,DILL_C):
	INSN_OUT(s, X_FORM(31, src, dest, 0, 51));
	ppc64le_lshi(s, dest, src, 24);
	ppc64le_rshai(s, dest, dest, 24);
	break;
    case CONV(DILL_F,DILL_S):
	INSN_OUT(s, HDR(0x2)|RD(src)|OP(0x34)|OPF(0xd1)|RS2(src));/*fstoi*/
	ppc64le_movf2i(s, dest, src);
	ppc64le_lshi(s, dest, src, 16);
	ppc64le_rshai(s, dest, dest, 16);
	break;
    case CONV(DILL_F,DILL_UC):
	INSN_OUT(s, HDR(0x2)|RD(src)|OP(0x34)|OPF(0xd1)|RS2(src));/*fstoi*/
	ppc64le_movf2i(s, dest, src);
	ppc64le_lshi(s, dest, src, 24);
	ppc64le_rshi(s, dest, dest, 24);
	break;
    case CONV(DILL_F,DILL_US):
	INSN_OUT(s, HDR(0x2)|RD(src)|OP(0x34)|OPF(0xd1)|RS2(src));/*fstoi*/
	ppc64le_movf2i(s, dest, src);
	ppc64le_lshi(s, dest, src, 16);
	ppc64le_rshi(s, dest, dest, 16);
	break;
    case CONV(DILL_F,DILL_U):
        {
	    int ret;
	    ppc64le_saverestore_floats(s, 0);
	    ret = dill_scallu(s, (void*)dill_ppc64le_hidden_ftou, "dill_ppc64le_hidden_ftou", "%f", src);
	    ppc64le_saverestore_floats(s, 1);
	    ppc64le_mov(s, DILL_UL, 0, dest, ret);
	}
	break;
	/* fallthrough */
    case CONV(DILL_F,DILL_UL):
        {
	    int ret;
	    ppc64le_saverestore_floats(s, 0);
	    ret = dill_scallul(s, (void*)dill_ppc64le_hidden_ftoul, "dill_ppc64le_hidden_ftoul", "%f", src);
	    ppc64le_saverestore_floats(s, 1);
	    ppc64le_mov(s, DILL_UL, 0, dest, ret);
	}
	break;
    case CONV(DILL_D,DILL_F):
	INSN_OUT(s, X_FORM(63, dest, 0, src, 12));
	break;
    case CONV(DILL_D,DILL_L):
	/* xscvdpsxds */
	INSN_OUT(s, XX2_FORM(60, src, src, 344));
	/* mfvsrc */
	INSN_OUT(s, X_FORM(31, src, dest, 0, 51));
	break;
    case CONV(DILL_D,DILL_I):
	/* xscvdpsxds */
	INSN_OUT(s, XX2_FORM(60, src, src, 344));
	/* mfvsrc */
	INSN_OUT(s, X_FORM(31, src, dest, 0, 51));
	break;
	break;
    case CONV(DILL_D,DILL_C):
    case CONV(DILL_D,DILL_UC):
	/* xscvdpsxds */
	INSN_OUT(s, XX2_FORM(60, src, src, 344));
	/* mfvsrc */
	INSN_OUT(s, X_FORM(31, src, dest, 0, 51));
	break;
	ppc64le_andi(s, dest, dest, 0xff);
	break;
    case CONV(DILL_D,DILL_S):
    case CONV(DILL_D,DILL_US):
	/* xscvdpsxds */
	INSN_OUT(s, XX2_FORM(60, src, src, 344));
	/* mfvsrc */
	INSN_OUT(s, X_FORM(31, src, dest, 0, 51));
	ppc64le_andi(s, dest, dest, 0xffff);
	break;
    case CONV(DILL_D,DILL_U):
        {
	    int ret;
	    ppc64le_saverestore_floats(s, 0);
	    ret = dill_scallu(s, (void*)dill_ppc64le_hidden_dtou, "dill_ppc64le_hidden_dtou", "%d", src);
	    ppc64le_saverestore_floats(s, 1);
	    ppc64le_mov(s, DILL_U, 0, dest, ret);
	}
	break;
    case CONV(DILL_D,DILL_UL):
        {
	    int ret;
	    ppc64le_saverestore_floats(s, 0);
	    ret = dill_scallul(s, (void*)dill_ppc64le_hidden_dtoul, "dill_ppc64le_hidden_dtoul", "%d", src);
	    ppc64le_saverestore_floats(s, 1);
	    ppc64le_mov(s, DILL_UL, 0, dest, ret);
	}
	break;
    case CONV(DILL_C,DILL_D):
    case CONV(DILL_S,DILL_D):
    case CONV(DILL_I,DILL_D):
	ppc64le_rshi(s, _gpr2, src, 0);
	src = _gpr2;
	/* fall through */
    case CONV(DILL_L,DILL_D):
	INSN_OUT(s, X_FORM(31, dest, src, 0, 179));
	INSN_OUT(s, XX2_FORM(60, dest, dest, 376));
	break;
    case CONV(DILL_UC,DILL_D):
    case CONV(DILL_US,DILL_D):
    case CONV(DILL_U,DILL_D):
	if (smi->stack_align == 8) { 
	    ppc64le_rshi(s, _gpr2, src, 0);
	    src = _gpr2;
	/* fall through */
	    /*	    ppc64le_pstorei(s, DILL_UL, 0, src, _fp, smi->conversion_word);
		    ppc64le_ploadi(s, DILL_D, 0, dest, _fp, smi->conversion_word);*/
	    INSN_OUT(s, HDR(0x2)|RD(dest)|OP(0x34)|OPF(0x88)|RS2(dest));/*fxtod*/
	    break;
	}
	/* fallthrough */
    case CONV(DILL_UL,DILL_D): 
        {
	    int ret;
	    ppc64le_saverestore_floats(s, 0);
	    ret = dill_scalld(s, (void*)dill_ppc64le_hidden_ultod, "dill_ppc64le_hidden_ultod", "%l", src);
	    ppc64le_saverestore_floats(s, 1);
	    ppc64le_mov(s, DILL_D, 0, dest, ret);
	}
	break;
    case CONV(DILL_C,DILL_F):
    case CONV(DILL_S,DILL_F):
    case CONV(DILL_I,DILL_F):
    case CONV(DILL_L,DILL_F):
	INSN_OUT(s, X_FORM(31, dest, src, 0, 179));
	INSN_OUT(s, XX2_FORM(60, dest, dest, 312));
	break;
    case CONV(DILL_UC,DILL_F):
    case CONV(DILL_US,DILL_F):
    case CONV(DILL_U,DILL_F):
	if (smi->stack_align == 8) { 
	    ppc64le_rshi(s, _gpr2, src, 0);
	    src = _gpr2;
	    /*	    ppc64le_pstorei(s, DILL_UL, 0, src, _fp, smi->conversion_word);
		    ppc64le_ploadi(s, DILL_D, 0, dest, _fp, smi->conversion_word);*/
	    INSN_OUT(s, HDR(0x2)|RD(dest)|OP(0x34)|OPF(0x84)|RS2(dest));/*fxtos*/
	    break;
	}
	/* fallthrough */
    case CONV(DILL_UL,DILL_F):
        {
	    int ret;
	    ppc64le_saverestore_floats(s, 0);
	    ret = dill_scalld(s, (void*)dill_ppc64le_hidden_ultof, "dill_ppc64le_hidden_ultof", "%l", src);
	    ppc64le_saverestore_floats(s, 1);
	    ppc64le_mov(s, DILL_D, 0, dest, ret);
	}
	break;
    case CONV(DILL_C,DILL_UL):
    case CONV(DILL_C,DILL_S):
    case CONV(DILL_C,DILL_L):
    case CONV(DILL_C,DILL_I):
    case CONV(DILL_C,DILL_U):
	ppc64le_lshi(s, dest, src, 24);
	ppc64le_rshai(s, dest, dest, 24);
	break;
    case CONV(DILL_S,DILL_C):
    case CONV(DILL_US,DILL_C):
    case CONV(DILL_I,DILL_C):
    case CONV(DILL_U,DILL_C):
    case CONV(DILL_L,DILL_C):
    case CONV(DILL_UL,DILL_C):
    case CONV(DILL_L,DILL_UC):
    case CONV(DILL_US,DILL_UC):
    case CONV(DILL_UL,DILL_UC):
    case CONV(DILL_I,DILL_UC):
    case CONV(DILL_U,DILL_UC):
    case CONV(DILL_S,DILL_UC):
	ppc64le_andi(s, dest, src, 0xff);
	break;
    case CONV(DILL_S,DILL_US):
	ppc64le_andi(s, dest, src, 0xffff);
	break;
    case CONV(DILL_S,DILL_L):
    case CONV(DILL_S,DILL_UL):
    case CONV(DILL_S,DILL_I):
    case CONV(DILL_S,DILL_U):
	ppc64le_lshi(s, dest, src, 16);
	ppc64le_rshai(s, dest, dest, 16);
	break;
    case CONV(DILL_US,DILL_I):
    case CONV(DILL_US,DILL_L):
    case CONV(DILL_US,DILL_U):
    case CONV(DILL_US,DILL_UL):
    case CONV(DILL_I,DILL_S):
    case CONV(DILL_U,DILL_S):
    case CONV(DILL_L,DILL_S):
    case CONV(DILL_UL,DILL_S):
    case CONV(DILL_I,DILL_US):
    case CONV(DILL_U,DILL_US):
    case CONV(DILL_L,DILL_US):
    case CONV(DILL_UL,DILL_US):
	ppc64le_lshi(s, dest, src, 16);
	ppc64le_rshi(s, dest, dest, 16);
	break;
    default:
	printf("Unknown case in ppc64le convert %d\n", CONV(from_type,to_type));
    }
}

extern void
ppc64le_compare(dill_stream s, int op, int type, int dest, int src1, int src2)
{
    int label = dill_alloc_label(s, "compare end");
    ppc64le_set(s, dest, 1);
    ppc64le_branch(s, op, type, src1, src2, label);
    ppc64le_set(s, dest, 0);
    dill_mark_label(s, label);
}

extern void
ppc64le_comparei(dill_stream s, int op, int type, int dest, int src, long imm)
{
    int label = dill_alloc_label(s, "compare end");
    ppc64le_set(s, dest, 1);
    ppc64le_branchi(s, op, type, src, imm, label);
    ppc64le_set(s, dest, 0);
    dill_mark_label(s, label);
}


static signed char op_BO[] = {
    0x0c, /* dill_eq_code = branch if eq bit is 1 */  
    0x04, /* dill_ge_code = branch if lt bit is 0 */
    0x0c, /* dill_gt_code = branch if gt bit is 1 */
    0x04, /* dill_le_code = branch if gt bit is 0 */
    0x0c, /* dill_lt_code = branch if lt bit is 1 */
    0x04, /* dill_ne_code = branch if eq bit is 0 */
};

static char op_BI[] = {
    0x02, /* dill_eq_code = test eq bit */
    0x00, /* dill_ge_code = test lt bit */
    0x01, /* dill_gt_code = test gt bit */
    0x01, /* dill_le_code = test gt bit */
    0x00, /* dill_lt_code = test lt bit */
    0x02, /* dill_ne_code = test eq bit */
};

extern void
ppc64le_branch(dill_stream s, int op, int type, int src1, int src2, int label)
{
    int cmp_op = 0;
    int L = 0;

    if (dill_type_size(s, type) == 8) L = 1;

    switch(type) {
    case DILL_F:
    case DILL_D:
	INSN_OUT(s, X_FORM(63, 0<<2, src1, src2, 0));
	dill_mark_branch_location(s, label);
	INSN_OUT(s, B_FORM(16, op_BO[op], op_BI[op], 0 /* target */, 0, 0));
	break;
    case DILL_UC:
    case DILL_US:
    case DILL_U:
    case DILL_UL:
	cmp_op = 32;   /* unsigned cmpl */
	/* falling through */
    default:
	INSN_OUT(s, X_FORM(31, ((0<<2)| L), src1, src2, cmp_op));
	dill_mark_branch_location(s, label);
	INSN_OUT(s, B_FORM(16, op_BO[op], op_BI[op], 0 /* target */, 0, 0));
    }
}

extern void 
ppc64le_jump_to_label(dill_stream s, unsigned long label)
{
    dill_mark_branch_location(s, label);
    INSN_OUT(s, 18<<26);
}

extern void ppc64le_jump_to_reg(dill_stream s, unsigned long reg)
{
    INSN_OUT(s, HDR(0x2)|OP(0x38)|RD(_gpr2)|RS1(reg)|IM|SIMM13(0x0));
    ppc64le_nop(c);
}

extern void ppc64le_jump_to_imm(dill_stream s, void * imm)
{
    ppc64le_nop(c);
}

extern void 
ppc64le_jal(dill_stream s, int return_addr_reg, int target)
{
    INSN_OUT(s, HDR(0x2)|OP(0x38)|RD(return_addr_reg)|RS1(target)|IM|SIMM13(0x0));
}

static void internal_push(dill_stream s, int type, int immediate, 
			  void *value_ptr)
{
    ppc64le_mach_info smi = (ppc64le_mach_info) s->p->mach_info;
    struct arg_info arg;
    int real_offset;

    arg.is_immediate = immediate;
    switch(type) {
    case DILL_C: case DILL_S:  case DILL_I: case DILL_L:
	arg.type = DILL_L;
	break;
    case DILL_UC: case DILL_US: case DILL_U: case DILL_UL:
	arg.type = DILL_UL;
	break;
    default:
	arg.type = type;
    }
	
    if (smi->cur_arg_offset < 8 * smi->stack_align) {
	arg.is_register = 1;
	if ((type == DILL_F) || (type == DILL_D)) {
	    arg.out_reg = _fpr1  + smi->cur_arg_offset/8;
	    arg.in_reg = _fpr1   + smi->cur_arg_offset/8;
	} else {
	    arg.in_reg = _gpr3 + smi->cur_arg_offset/smi->stack_align;
	    arg.out_reg = _gpr3 + smi->cur_arg_offset/smi->stack_align;
	}
    } else {
	if ((smi->stack_align == 8) && ((type == DILL_F) || (type == DILL_D)) &&
	    (smi->cur_arg_offset <= 8 * smi->stack_align)) {
	    /* floating arg can go in a float reg, but not int reg */
	    arg.is_register = 1;
	    arg.out_reg = _fpr0  + smi->cur_arg_offset/4;
	    arg.in_reg = -1;
	} else {
	    arg.is_register = 0;
	}
    }

    arg.offset = smi->cur_arg_offset;
    smi->cur_arg_offset += 
	roundup(type_info[(int)arg.type].size, smi->stack_align);
    real_offset = arg.offset + 32;
    if (arg.is_register == 0) {
	/* store it on the stack only */
	if (arg.is_immediate) {
	    if (type == DILL_F) {
		float f = (float) *(double*)value_ptr;
		ppc64le_set(s, _gpr2, *(int*)&f);
	    } else {
		ppc64le_set(s, _gpr2, *(long*)value_ptr);
	    }
	    ppc64le_pstorei(s, arg.type, 0, _gpr2, _sp, real_offset);
	} else {
	    ppc64le_pstorei(s, arg.type, 0, *(int*)value_ptr, _sp, 
			    real_offset);
	}
    } else {
	if ((type != DILL_F) && (type != DILL_D)) {
	    if (arg.is_immediate) {
		ppc64le_set(s, arg.out_reg, *(long*)value_ptr);
	    } else {
		ppc64le_mov(s, type, 0, arg.out_reg, *(int*) value_ptr);
	    }
	} else {
	    if (arg.is_immediate) {
		if ((type == DILL_F) || (type == DILL_D)) {
		    /* set appropriate register */
		    ppc64le_setf(s, type, 0, arg.out_reg, 
				 *(double*)value_ptr);
		} else {
		    ppc64le_set(s, arg.out_reg, *(int*)value_ptr);
		}
	    } else {
		/* move to the appropriate float reg */
		ppc64le_mov(s, type, 0, arg.out_reg, *(int*)value_ptr);
	    }
	    if (arg.in_reg != -1) {
		/* put value in int regs too */
		if (type == DILL_D) {
		    ppc64le_movd2i(s, arg.in_reg, arg.out_reg);
		} else {
		    ppc64le_movf2i(s, arg.in_reg, arg.out_reg);
		}
	    } else {
		/* put it on the stack as well */
		ppc64le_pstorei(s, arg.type, 0, arg.out_reg, _sp,
				real_offset);
	    }
	}
    }
}

static void push_init(dill_stream s)
{
    ppc64le_mach_info smi = (ppc64le_mach_info) s->p->mach_info;
    smi->cur_arg_offset = 0;
}

extern void ppc64le_push(dill_stream s, int type, int reg)
{
    if ((type == DILL_V) && (reg <= -1)) {
	push_init(s);
    } else {
	internal_push(s, type, 0, &reg);
    }
}

extern void ppc64le_pushi(dill_stream s, int type, long value)
{
    internal_push(s, type, 1, &value);
}

extern void ppc64le_pushpi(dill_stream s, int type, void *value)
{
    internal_push(s, type, 1, &value);
}

extern void ppc64le_pushfi(dill_stream s, int type, double value)
{
    internal_push(s, type, 1, &value);
}

extern int ppc64le_calli(dill_stream s, int type, void *xfer_address, const char *name)
{
    int caller_side_ret_reg = _gpr3;

    /* save temporary registers */
    dill_mark_call_location(s, name, xfer_address);
    /* the next 5 ops will load a location into R12 */
    ppc64le_set(s, _gpr12, 0x1234567812345678);
/*    ppc64le_nop(s);
    ppc64le_nop(s);
    ppc64le_nop(s);
    ppc64le_nop(s);
    ppc64le_nop(s);*/
    /* mtctr r12  (store jump addrss from r12 into control reg */
    INSN_OUT(s, XFX_FORM(31, _gpr12, 0x120, 467));
    /* std r2, 24(r1)    (save our r2 value) */
    ppc64le_pstorei(s, DILL_L, 0, _gpr2, _gpr1, 24);
    /* bctrl (branch and link to control reg) */
    INSN_OUT(s, 0x4e800421);

    /* restore temporary registers */
    /* ld r2, 24(r1)    (load our r2 value back) */
    ppc64le_ploadi(s, DILL_L, 0, _gpr2, _gpr1, 24);

    if ((type == DILL_D) || (type == DILL_F)) {
	caller_side_ret_reg = _fpr0;
    }
    push_init(s);
    return caller_side_ret_reg;
}

extern int ppc64le_callr(dill_stream s, int type, int src)
{
    int caller_side_ret_reg = _gpr3;

    /* save temporary registers */
    /*    ppc64le_jal(s, _o7, src);
    ppc64le_nop(s);*/
    /* restore temporary registers */
    if ((type == DILL_D) || (type == DILL_F)) {
	caller_side_ret_reg = _fpr0;
    }
    push_init(s);
    return caller_side_ret_reg;
}

extern void
ppc64le_branchi(dill_stream s, int op, int type, int src, long imm, int label)
{
    switch(type) {
    case DILL_F:
    case DILL_D:
	fprintf(stderr, "Shouldn't happen\n");
	break;
    default:
	if  (((long)imm) >= 32767 || ((long)imm) < -32768) {
	    ppc64le_set(s, _gpr0, imm);
	    ppc64le_branch(s, op, type, src, _gpr0, label);
	} else {
	    INSN_OUT(s, D_FORM(11, 0<<2 | 1, src, imm));
	    dill_mark_branch_location(s, label);
	    INSN_OUT(s, B_FORM(16, op_BO[op], op_BI[op], 0 /* target */, 0, 0));
	}
    }
}

extern void ppc64le_ret(dill_stream s, int data1, int data2, int src)
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
	if (src != _gpr3) ppc64le_int_mov(s, _gpr3, src);
	break;
    case DILL_F:
	if (src != _fpr0) ppc64le_movf(s, _fpr1, src);
	break;
    case DILL_D:
	if (src != _fpr0) ppc64le_movd(s, _fpr1, src);
	break;
    }
    ppc64le_simple_ret(s);
}

extern void ppc64le_reti(dill_stream s, int data1, int data2, long imm)
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
	ppc64le_set(s, _gpr3, imm);
	break;
    case DILL_F:
    case DILL_D:
	break;/* no return immediate of floats */
    }
    ppc64le_simple_ret(s);
}

static void
ppc64le_data_link(dill_stream s)
{
/*    struct branch_table *t = &s->p->branch_table;
    int i;
    for (i=0; i < t->data_mark_count; i++) {
	int label = t->data_marks[i].label;
	void *label_addr = t->label_locs[label] + (char*)s->p->code_base;
	*t->data_marks[i].addr = label_addr;
	}*/
}

static void
ppc64le_branch_link(dill_stream s)
{
    struct branch_table *t = &s->p->branch_table;
    int i;

    for(i=0; i< t->branch_count; i++) {
	int label = t->branch_locs[i].label;
	int label_offset = t->label_locs[label] - t->branch_locs[i].loc;
	int *branch_addr = (int*)((char *)s->p->code_base + 
				  t->branch_locs[i].loc);
	if ((*branch_addr & 0xfa000000) == (18<<26)) {
	    /* unconditional branch   I-form, adjust LI field */
	    *branch_addr &= 0xfa000000;
	    *branch_addr |= (label_offset & 0x3fffffc);
	} else {
	    /* conditional branch   B-form, adjust BD field */
	    *branch_addr &= 0xffff0000;
	    *branch_addr |= (label_offset & 0xfffc);
	}
    }
}

/*
 * on 64-bit solaris 8, we need a procedure linkage table to manage 
 * calls to DLLs in an address range that is typically more than 32 bits
 * away from malloc'd memory.  We emit a PLT that is basically a set_reg, 
 * then jump_through_reg for each routine.  Later, during call linkage, 
 * we'll call to the PLT entry rather than directly to the routine.
 */
static void
ppc64le_PLT_emit(dill_stream s, int force_PLT)
{
    call_t *t = &s->p->call_table;
    int i;

    for(i=0; i< t->call_count; i++) {
	int *call_addr = (int*) ((unsigned long)s->p->code_base + 
				 t->call_locs[i].loc);
	long call_offset = (unsigned long)t->call_locs[i].xfer_addr - 
	    (unsigned long)call_addr;
	int jump_value;

        /* div addr diff by 4 for ppc64le offset value */
	call_offset = call_offset >> 2;
	jump_value = (char*)s->p->cur_ip - (char*)call_addr;
	call_offset = call_offset >> 30;
	t->call_locs[i].mach_info = (void*)0;
	if (((call_offset != 0) && (call_offset != -1)) || force_PLT) {

	    int plt_offset = (char*)s->p->cur_ip - (char*)s->p->code_base;
	    t->call_locs[i].mach_info = (void*)1;

	    ppc64le_nop(s);
	    ppc64le_nop(s);
	    ppc64le_nop(s);
	    ppc64le_nop(s);
	    ppc64le_nop(s);
	    ppc64le_nop(s);

/*	    ppc64le_set(s, _gpr2, (unsigned long)t->call_locs[i].xfer_addr);*/
	    ppc64le_jump_to_reg(s, _gpr2);
	    ppc64le_nop(s);

	    /* fixup call to reference just-generated PLT code*/
	    call_addr = (int*) ((unsigned long)s->p->code_base + 
				t->call_locs[i].loc);
	    t->call_locs[i].loc = plt_offset;
	    jump_value = jump_value >> 2;
	    *call_addr &= 0xc0000000;
	    *call_addr |= (jump_value & 0x3fffffff);
	}
    }
}

extern void ppc64le_rt_call_link(char *code, call_t *t, int force_plt);

static void
ppc64le_call_link(dill_stream s)
{
    call_t *t = &s->p->call_table;

    ppc64le_rt_call_link(s->p->code_base, t, /* don't force plt */ 0);
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

extern void
ppc64le_end(s)
dill_stream s;
{
    ppc64le_simple_ret(s);
    ppc64le_emit_proc_epilogue(s);
    ppc64le_emit_proc_prologue(s);
    ppc64le_PLT_emit(s, 0);   /* must be done before linking */
    ppc64le_branch_link(s);
    ppc64le_call_link(s);
    ppc64le_data_link(s);
    ppc64le_flush(s->p->code_base, s->p->code_limit);
}

static void
ppc64le_simple_ret(dill_stream s) 
{
    ppc64le_mach_info smi = (ppc64le_mach_info) s->p->mach_info;
    ppc64le_jump_to_label(s, smi->epilogue_label);
}

/*
 * save (spill) registers to memory or load (fill) registers from memory
 */
static void
ppc64le_spill_fill_vars(dill_stream s, int action)
{
    ppc64le_mach_info smi = (ppc64le_mach_info) s->p->mach_info;
    int reg;
    int ireg_start = _gpr14;
    int ireg_end = _gpr31;
    int freg_start = _fpr14;
    int freg_end = _fpr31;

    for(reg = ireg_start; reg <= ireg_end; reg++) {
	if (dill_wasused(&s->p->var_i, reg)) {
	    int offset = int_reg_store_offset(s, reg);
	    if (action == SPILL) {
		ppc64le_pstorei(s, DILL_L, 0, reg, _sp, offset);
	    } else {
		ppc64le_ploadi(s, DILL_L, 0, reg, _sp, offset);
	    }
	}
    }
    for(reg = freg_start; reg <= freg_end; reg++) {
	if (dill_wasused(&s->p->var_f, reg)) {
	    int offset = float_reg_store_offset(s, reg);
	    if (action == SPILL) {
		ppc64le_pstorei(s, DILL_D, 0, reg, _sp, offset);
	    } else {
		ppc64le_ploadi(s, DILL_D, 0, reg, _sp, offset);
	    }
	}
    }
}

extern void
ppc64le_package_end(s)
dill_stream s;
{
    int force_plt = 0;
    ppc64le_simple_ret(s);
#if defined(HOST_PPC64LEV9)
    force_plt = 1;
#endif
    ppc64le_PLT_emit(s, force_plt);   /* must be done before linking */
    ppc64le_branch_link(s);
}

extern void *
ppc64le_clone_code(s, new_base, available_size)
dill_stream s;
void *new_base;
int available_size;
{
    int size = dill_code_size(s);
    if (available_size < size) {
	return NULL;
    }
    void *old_base = s->p->code_base;
    void *native_base = s->p->code_base;
    if (native_base == NULL) native_base = s->p->native.code_base;
    memcpy(new_base, native_base, size);
    s->p->code_base = new_base;
    s->p->cur_ip = (void*)((long)new_base + size);
    s->p->fp = new_base;
    ppc64le_branch_link(s);
    ppc64le_call_link(s);
    ppc64le_data_link(s);
    ppc64le_flush(s->p->code_base, s->p->code_limit);
    s->p->code_base = old_base;
    s->p->cur_ip = (void*)((long) old_base + size);
    s->p->fp = old_base;
    while (*(int*)new_base == 0x10000) {
	/* skip UNIMPs */
	new_base = (void*) ((long) new_base + 4);
    }
    return new_base;
}

extern void
ppc64le_pset(dill_stream s, int type, int junk, int dest, long imm)
{
    ppc64le_set(s, dest, imm);
}	

extern void
ppc64le_setp(dill_stream s, int type, int junk, int dest, void *imm)
{
    ppc64le_mach_info smi = (ppc64le_mach_info) s->p->mach_info;
    union {
	void *a;
	int i;
	long l;
    } a;
    a.a = imm;
    if (smi->stack_align == 4) {
	ppc64le_set(s, dest, a.i);
    } else {
	ppc64le_set(s, dest, a.l);
    }
}

extern void
ppc64le_setf(dill_stream s, int type, int junk, int dest, double imm)
{
    union {
	float f;
	int i;
    } a;
    union {
	double d;
	long l;
	int i[2];
    } b;
    ppc64le_mach_info smi = (ppc64le_mach_info) s->p->mach_info;
    if (type == DILL_F) {
	a.f = (float) imm;
	ppc64le_set(s, _gpr2, a.i);
	ppc64le_movi2f(s, dest, _gpr2);
    } else if (smi->stack_align == 4) {
	b.d = imm;
	ppc64le_set(s, _gpr2, b.i[0]);
	ppc64le_movi2f(s, dest, _gpr2);
	ppc64le_set(s, _gpr2, b.i[1]);
	ppc64le_movi2f(s, dest+1, _gpr2);
    } else {
	/* double ppc64lev9 */
	b.d = imm;
	ppc64le_set(s, _gpr2, b.l);
	ppc64le_movi2d(s, dest, _gpr2);
    }
}	

extern void
ppc64le_set(s, r, val)
dill_stream s;
int r;
long val;
{
    if ((val > 2147483647) || (val < -2147483647) ) {
	/* need to set all 64 bits */
	ppc64le_set(s, r, ((val >> 32) & 0xffffffff));

	/* left shift the 32 we just set */
	ppc64le_shiftimm_arith(s, 21 /*lsh*/, DILL_L, r, r, 32);

	// or immediate shifted
	INSN_OUT(s, D_FORM(25, r, r, hi16(val)));
	// or immediate 
	INSN_OUT(s, D_FORM(24, r, r, lo16(val)));
    } else if ((((long)val) > 32767) || (((long)val) <= -32768)) {
	/* need to set low 32 bits */
	// add immediate shifted
	INSN_OUT(s, D_FORM(15, r, 0, hi16(val)));
	// or immediate 
	INSN_OUT(s, D_FORM(24, r, r, lo16(val)));
    } else {	
	// add immediate 
	INSN_OUT(s, D_FORM(14, r, 0, lo16(val)));
    }	
}

extern void ppc64le_bswap(s, junk, typ, dest, src)
dill_stream s;
int junk;
int typ;
int dest;
int src;
{
    switch (typ) {
    case DILL_S:
    case DILL_US:
	/* stwu  src, -16(r1) */
	INSN_OUT(s, D_FORM(45, src, _gpr1, -16 & 0xffff));
	/* lwbru dest, (r1) */
	INSN_OUT(s, X_FORM(31, dest, 0, _gpr1, 790));
	break;
    case DILL_I:
    case DILL_U:
	/* stwu  src, -16(r1) */
	INSN_OUT(s, D_FORM(37, src, _gpr1, -16 & 0xffff));
	/* lwbru dest, (r1) */
	INSN_OUT(s, X_FORM(31, dest, 0, _gpr1, 534));
	break;
    case DILL_L:
    case DILL_UL:
	/* stdu  src, -16(r1) */
	INSN_OUT(s, D_FORM(62, src, _gpr1, (-16 & 0xfffc) | 0x1));
	/* lwbru dest, (r1) */
	INSN_OUT(s, X_FORM(31, dest, 0, _gpr1, 532));
	break;
    case DILL_F:
	/* stfsu  src, -16(r1) */
	INSN_OUT(s, D_FORM(53, src, _gpr1, -16 & 0xffff));
	/* lwbru _gpr0, (r1) */
	INSN_OUT(s, X_FORM(31, _gpr0, 0, _gpr1, 534));
	/* stw  _gpr0, (r1) */
	INSN_OUT(s, D_FORM(36, _gpr0, _gpr1, 0));
	/* lfs dest, (r1) */
	INSN_OUT(s, D_FORM(49, dest, _gpr1, 0));
	break;
    case DILL_D:
	/* stfdu  src, -16(r1) */
	INSN_OUT(s, D_FORM(55, src, _gpr1, -16 & 0xffff));
	/* lwbru dest, (r1) */
	INSN_OUT(s, X_FORM(31, _gpr0, 0, _gpr1, 532));
	/* addi r1, r1, 16 */
	/* std  _gpr0, (r1) */
	INSN_OUT(s, D_FORM(62, _gpr0, _gpr1, 0));
	/* lfd dest, (r1) */
	INSN_OUT(s, D_FORM(50, dest, _gpr1, 0));
	break;
    }
    /* addi r1, r1, 16 */
    INSN_OUT(s, D_FORM(14, _gpr1, _gpr1, 16));
}

#define bit_R(x) ((unsigned long)1<<x)

extern void
ppc64le_reg_init(dill_stream s)
{
    /* 
     *  These PPC integer registers (R14-r31) are non-volatile must be
     *  preserved across calls.  So if we use them we must save/restore them
     *  in prologue/epilog, but we don't have to worry about them across
     *  calls that we make.
     *
     *	r0        Volatile register used in function prologs
     *  r1        Stack frame pointer
     *  r2        TOC pointer
     *  r3        Volatile parameter and return value register
     *  r4-r10    Volatile registers used for function parameters
     *  r11       Volatile register used in calls by pointer and as an
     *            environment pointer for languages which require one
     *  r12       Volatile register used for exception handling and glink code
     *	r13       Reserved for use as system thread ID
     *	r14-r31   Nonvolatile registers used for local variables
     *
     * Nothing is a true temporary because of the mandatory use in parameter 
     * passing.
     * The TOC stuff isn't really applicable to DCG code, so we're simply
     * not going to touch r2.  We will use r0 as a very-short-term temporary.
     */

    s->p->var_i.init_avail[0] = (bit_R(_gpr14)|bit_R(_gpr15)|
				 bit_R(_gpr16)|bit_R(_gpr17)|bit_R(_gpr18)|
				 bit_R(_gpr19)|bit_R(_gpr20)|bit_R(_gpr21)|
				 bit_R(_gpr22)|bit_R(_gpr23)|bit_R(_gpr24)|
				 bit_R(_gpr25)|bit_R(_gpr26)|bit_R(_gpr27)|
				 bit_R(_gpr28)|bit_R(_gpr29)|bit_R(_gpr30)|
				 bit_R(_gpr31));
    s->p->var_i.members[0] = s->p->var_i.init_avail[0];
    s->p->tmp_i.init_avail[0] = 0;
    s->p->tmp_i.members[0] = s->p->tmp_i.init_avail[0];

    

    /* 
     *  These PPC float registers (F14-F31) are non-volatile must be
     *  preserved across calls.  So if we use them we must save/restore them
     *  in prologue/epilog, but we don't have to worry about them across
     *  calls that we make.
     *
     *  Additionally, F0 is a volatile scratch reg and F1-F13 are
     *  potentially required for parameter passing, so we won't list
     *  anything as a "temporary register" because of that mandatory use.
     */
    s->p->var_f.init_avail[0] = (bit_R(_fpr14)|bit_R(_fpr15)|bit_R(_fpr16)|
				 bit_R(_fpr17)|bit_R(_fpr18)|bit_R(_fpr19)|
				 bit_R(_fpr20)|bit_R(_fpr21)|bit_R(_fpr22)|
				 bit_R(_fpr23)|bit_R(_fpr24)|bit_R(_fpr25)|
				 bit_R(_fpr26)|bit_R(_fpr27)|bit_R(_fpr28)|
				 bit_R(_fpr29)|bit_R(_fpr30)|bit_R(_fpr31));
    s->p->var_f.members[0] = s->p->var_f.init_avail[0];
    s->p->tmp_f.init_avail[0] = 0;
    s->p->tmp_f.members[0] = s->p->tmp_f.init_avail[0];
}

extern void*
gen_ppc64le_mach_info(s, bit64)
dill_stream s;
int bit64;
{
    ppc64le_mach_info smi = malloc(sizeof(*smi));
    if (s->p->mach_info != NULL) {
	free(s->p->mach_info);
	s->p->mach_info = NULL;
	s->p->native.mach_info = NULL;
    }
    ppc64le_reg_init(s);
    smi->act_rec_size = 0;
    smi->conversion_word = 0;
    smi->cur_arg_offset = 0;
    if (bit64) {
	smi->stack_align = 8; /* 8 for ppc64le */
	smi->stack_constant_offset = 2047; 
    } else {
	smi->stack_align = 4; /* 4 for ppowerpc */
	smi->stack_constant_offset = 0;
    }
    smi->gp_save_offset = (16 + 1 + 6 + 19 /* args */) * smi->stack_align; /*184;*/
    smi->fp_save_offset = smi->gp_save_offset + 8 * smi->stack_align;
    smi->fp_save_end = smi->fp_save_offset + 32 * smi->stack_align + 16;
    return smi;
}

#if defined(HAVE_DIS_ASM_H) && !defined(NO_DISASSEMBLER)
/* GENERIC BINUTILS DISASSEMBLER */
#include "dis-asm.h"

#define MAXLENGTH (1<<23) /* Max length of function that can be disassembled */

extern int
ppc64le_init_disassembly_info(dill_stream s, void * ptr)
{
    struct disassemble_info *i = ptr;
#ifdef INIT_DISASSEMBLE_INFO_THREE_ARG
    INIT_DISASSEMBLE_INFO(*i, stdout,fprintf);
#else
    INIT_DISASSEMBLE_INFO(*i, stdout);
#endif
    i->mach = bfd_mach_ppc;
    i->arch = bfd_arch_powerpc;
    if (s->p->code_base != NULL) {
	i->buffer = (bfd_byte *)s->p->code_base;
	i->buffer_vma = (bfd_vma)(long)s->p->code_base;
    } else {
	i->buffer = (bfd_byte *)s->p->native.code_base;
	i->buffer_vma = (bfd_vma)(long)s->p->native.code_base;
    }
    i->buffer_length = MAXLENGTH;
    disassemble_init_powerpc(i);
#ifdef HAVE_PRINT_INSN_BIG_POWERPC
    return 1;
#else
    return 0;
#endif
}

extern int
ppc64le_print_insn(dill_stream s, void *info_ptr, void *insn)
{
#ifdef HAVE_PRINT_INSN_BIG_POWERPC
    return print_insn_little_powerpc((unsigned long) insn, (disassemble_info*)info_ptr);
#else
    return 0;
#endif
}
#else
extern int
ppc64le_init_disassembly_info(dill_stream s, void * ptr){return 0;}
extern int ppc64le_print_insn(dill_stream s, void *info_ptr, void *insn){return 0;}
#endif

extern void
ppc64le_print_reg(dill_stream s, int typ, int reg)
{
    switch(typ) {
    case DILL_C: case DILL_UC:
    case DILL_S: case DILL_US:
    case DILL_I: case DILL_U: case DILL_L: case DILL_UL:
	if (reg == _sp) {
	    printf("sp");
	    return;
	} else if (reg <= _gpr31) {
	    printf("g%d\n", reg - _gpr0);
	    return;
	}
	break;
    case DILL_F: case DILL_D:
	printf("F%d", reg);
	return;
    }
    printf("NoReg(%d)", reg);
}

extern int
ppc64le_count_insn(dill_stream s, int start, int end)
{
    return (end - start)>>2;
}

extern void 
ppc64le_XFORM2_farith(dill_stream c, int op3, int op, int dest, int src) 
{
    INSN_OUT(c, X_FORM(63, dest, 0, src, op3));
}
