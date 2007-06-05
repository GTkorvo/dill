#include <assert.h>
#include <stdio.h>
#include "dill.h"
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) 
{ 
    int verbose = 0;
    int i;
    for (i=1; i < argc; i++) {
	if (strcmp(argv[i], "-v") == 0) {
	    verbose++;
	}
    }

    {
	dill_stream s = dill_create_raw_stream();
	int (*func)();
	int result;
	char *pkg;
	int pkg_len;
	dill_start_simple_proc(s, "foo", DILL_I);
	dill_retii(s, 5);
	pkg = dill_finalize_package(s, &pkg_len);

	dill_free_stream(s);

	func = (int(*)()) dill_package_stitch(pkg);

	result = func();
	if (result != 5) {
	    printf("Test 1 failed, got %d instead of 5\n", result);
	    exit(1);
	} else if (verbose) {
	    printf("Test 1 passed, got %d\n", result);
	}
	free(pkg);
    }

    {
	dill_stream s = dill_create_raw_stream();
	unsigned expected_result;
	unsigned result;
	unsigned(*func)(unsigned,unsigned);
	char *pkg;
	int pkg_len;
	unsigned s1u, s2u;

	dill_reg	rdu;
	/* reg <- (reg % reg) */
	if (verbose) printf(" - modu\n");
	s1u = rand() - rand();
	if(!(s2u = rand() - rand()))
	    s2u = rand() + 1;
        dill_start_proc(s, "modu",  DILL_U, "%u%u");
	dill_raw_getreg(s, &rdu, DILL_U, DILL_TEMP);

	dill_modu(s, rdu, dill_param_reg(s, 0), dill_param_reg(s, 1));
	dill_retu(s, rdu);

        pkg = dill_finalize_package(s, &pkg_len);
	dill_free_stream(s);

	func = (unsigned (*)(unsigned, unsigned)) dill_package_stitch(pkg);
	result = func(s1u, s2u);
        expected_result = (s1u % s2u);
	if (expected_result != result) {
	    printf("Failed test for %u modu %u, expected %u, got %u\n", 
		   s1u, s2u, expected_result, result);
	} else if (verbose) {
	    printf("Test for %u modu %u, passed.  Expected %u, got %u\n", 
		   s1u, s2u, expected_result, result);
	}
	free(pkg);
    }
    return 0;
}
