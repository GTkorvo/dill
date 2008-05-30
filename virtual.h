
#define INSN_OUT(c, i) \
if ((((char*)c->p->cur_ip) + sizeof(virtual_insn)) >= (char*)c->p->code_limit) {\
   extend_dill_stream(c);\
}\
*(virtual_insn*)c->p->cur_ip = i;\
if (c->dill_debug) dump_cur_dill_insn(c);\
c->p->cur_ip = ((char*)c->p->cur_ip)+ sizeof(virtual_insn)

typedef enum {iclass_arith3, iclass_arith3i, iclass_arith2, iclass_ret, iclass_convert, iclass_loadstore, iclass_loadstorei, iclass_set, iclass_setf, iclass_mov, iclass_reti, iclass_branch, iclass_branchi, iclass_jump_to_label, iclass_special, iclass_jump_to_reg, iclass_jump_to_imm, iclass_push, iclass_pushi, iclass_pushf, iclass_call, iclass_lea, iclass_compare, iclass_nop} insn_class;
typedef struct bitv {
    short len;
    char vec[2];
} *bit_vec;

typedef struct basic_block {
    int start;
    int end;
    int label;
    int end_branch_label;
    int fall_through;
    int pred_count;
    int *pred_list;
    int succ_count;
    int *succ_list;
    bit_vec regs_used;
    bit_vec regs_defined;
    bit_vec live_at_end;
    short *reg_assigns;
    int visited;
    int loop_depth;
    int is_loop_start;
    int is_loop_end;
} *basic_block;
    
typedef struct mach_info {
    arg_info_list arg_info;
    int bbcount;
    basic_block bblist;
} *virtual_mach_info;

struct arith3 {
    unsigned short dest;
    unsigned short src1;
    unsigned short src2;
};

struct arith3i {
    unsigned short dest;
    unsigned short src;
    union {
	long imm;
	void *imm_a;
    }u;
};

struct arith2 {
    unsigned short dest;
    unsigned short src;
};

struct arith1 {
    unsigned short src;
};

struct setf {
    unsigned short dest;
    double imm;
};

struct branch {
    unsigned short src1;
    unsigned short src2;
    unsigned short label;
};

struct branchi {
    unsigned short src;
    unsigned short label;
    void *imm_a;
    long imm_l;
};

struct special {
    special_operations type;
    long param;
};

union operands {
    struct arith3 a3;
    struct arith3i a3i;
    struct arith2 a2;
    struct arith1 a1;
    struct setf sf;
    struct branch br;
    struct branchi bri;
    struct special spec;
};

typedef struct {
    char class_code;
    char insn_code;
    union operands opnds;
} virtual_insn;

extern int dill_type_of(dill_stream c, int vreg);
extern void
virtual_proc_start(dill_stream c, char *subr_name, int arg_count,
		   arg_info_list args, dill_reg *arglist);
extern void virtual_end(dill_stream c);
extern void virtual_package_end(dill_stream c);
extern int virtual_print_insn(dill_stream c, void *info_ptr, void *i);
