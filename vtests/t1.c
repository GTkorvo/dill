#include "../config.h"
#include "stdio.h"
#include "malloc.h"
#include "unistd.h"

#include "dill.h"
#ifdef USE_MMAP_CODE_SEG
#include "sys/mman.h"
#endif

static int verbose = 0;

void a () {
    dill_stream s = dill_create_stream();
    char *target;
    dill_reg a,b,p3,d,e,f,g,h,i,j,w,z;
    dill_exec_ctx ec;
     int (*ip)();

     dill_start_proc(s, "a_gen", DILL_I, "%EC%i%i");

     a = dill_vparam(s, 1);
     b = dill_vparam(s, 2);
     p3 = dill_getreg(s, DILL_I);
     d = dill_getreg(s, DILL_I);
     e = dill_getreg(s, DILL_I);
     f = dill_getreg(s, DILL_I);
     g = dill_getreg(s, DILL_I);
     h = dill_getreg(s, DILL_I);
     i = dill_getreg(s, DILL_I);
     j = dill_getreg(s, DILL_I);
     z = dill_getreg(s, DILL_I);
     w = dill_getreg(s, DILL_I);

     dill_addii(s, p3, a, 5);
     dill_addi(s, d, a, b);
     dill_addi(s, e, d, p3);
     dill_movi(s, f, e);

     p3 = dill_getreg(s, DILL_I);
     d = dill_getreg(s, DILL_I);
     e = dill_getreg(s, DILL_I);
     dill_addii(s, p3, a, 5);
     dill_addi(s, d, a, b);
     dill_addi(s, e, d, p3);

     dill_addi(s, f, f, e);
     dill_reti(s, f);
     ip = (int(*)())dill_finalize(s);

#ifdef USE_MMAP_CODE_SEG
     {
	 int size = dill_code_size(s);
	 static unsigned long ps = -1;
	 if (ps == -1) {
	     ps = (getpagesize ());
	 }
	 if (ps > size) size = ps;
	 target = (void*)mmap(0, size, 
			      PROT_EXEC | PROT_READ | PROT_WRITE, 
			      MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
     }
     if (target == (void*)-1) perror("mmap");
#else
     target = (void*)malloc(dill_code_size(s));
#endif
     if (verbose) dill_dump(s);

     ec = dill_get_exec_context(s);
     printf("**18=%d\n", (*ip)(ec, 1, 2));
     dill_free_stream(s);
}

int 
main(int argc, char **argv)
{
    if (argc > 1) verbose++;
    printf("########## A\n");
    a();
    printf("########## end\n");
    return 0;
}
