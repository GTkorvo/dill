/* Test that save/restore and locals do not overlap in the activation record. */
#include <assert.h>
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>

#include "../config.h"
#include "dill.h"
#ifdef USE_MMAP_CODE_SEG
#include "sys/mman.h"
#endif

void * mk_test(dill_stream c) {
	int i, regs, fregs;
	int l[100];
	int abortl1;
	int abortl2;
	int abortl3;

	dill_reg r[32];
	dill_reg fr[32];

	dill_start_proc(c, "foo", DILL_I, "%i%d");

	abortl1 = dill_alloc_label(c);
	abortl2 = dill_alloc_label(c);
	abortl3 = dill_alloc_label(c);

	for(i = 0; i < 5 && dill_raw_getreg(c, &r[i], DILL_I, DILL_TEMP); i++) {
/*		(void)("allocated register %d\n", i);*/
		dill_seti(c, r[i], i);
		dill_savei(c, r[i]); 
	}
	regs = i;

	for(i = 0; dill_raw_getreg(c, &fr[i], DILL_D, DILL_TEMP); i++) {
/*		(void)("allocated register %d\n", i);*/
		dill_setd(c, fr[i], (double)i);
		dill_saved(c, fr[i]); 
	}
	fregs = i;

	for(i = 0; i < 10; i++) {
		l[i] = dill_local(c, DILL_I);
			
		dill_seti(c, dill_param_reg(c,0), 100 + i);
		dill_stii(c, dill_param_reg(c,0), dill_lp(c), l[i]);
	}

	/* call a procedure, then verify that everything is in place. */
	dill_scallv(c, (void*)printf, "%P", "hello world!\n");


	for(i = 0; i < regs; i++) {
		dill_restorei(c, r[i]); 
		dill_bneii(c, r[i], i, abortl1);
	}

	for(i = 0; i < fregs; i++) {
/*		(void)("allocated register %d\n", i);*/
		dill_restored(c, fr[i]); 
		dill_setd(c, dill_param_reg(c,1), (double)i);
		dill_bned(c, fr[i], dill_param_reg(c,1), abortl2);
	}

	for(i = 0; i < 10; i++) {
		dill_ldii(c, dill_param_reg(c, 0), dill_lp(c), l[i]);
		dill_bneii(c, dill_param_reg(c, 0), 100 + i, abortl3);
	}
	dill_retii(c, 0);

	dill_mark_label(c, abortl1);
	dill_retii(c, 1);		/* failure. */
	dill_mark_label(c, abortl2);
	dill_retii(c, 2);		/* failure. */
	dill_mark_label(c, abortl3);
	dill_retii(c, 3);		/* failure. */

	return dill_end(c);
}

int main() { 
	int (*ip)();
	dill_stream c = dill_create_raw_stream();
	int ret;
	char *target;

	ip = (int (*)()) mk_test(c);
	ret = ip();

	if(ret == 0) {
		printf("success!\n");
	} else {
		dill_dump(c);
		printf("failure at point %d!\n", ret);
	}
	ip = (int (*)()) mk_test(c);
	ret = ip();
	if(ret == 0)
		printf("success!\n");
	else {
		dill_dump(c);
		printf("failure at point %d (second)\n", ret);
	}
#ifdef USE_MMAP_CODE_SEG
	{
	    int size = dill_code_size(c);
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
	target = (void*)malloc(dill_code_size(c));
#endif
	ip = (int (*)()) dill_clone_code(c, target, dill_code_size(c));
	dill_free_context(c);
	ret = ip();
	if(ret == 0)
		printf("success!\n");
	else {
		dill_dump(c);
		printf("failure at point %d (third)\n", ret);
	}

	return 0;
}
