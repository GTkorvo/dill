#include <assert.h>
#include <stdio.h>
#include "dill.h"
#include <stdlib.h>

int main(int argc, char **argv) 
{ 
    dill_stream c = dill_create_raw_stream();
    int (*func)();
    int verbose = 0;

    {
	int result;
	dill_start_simple_proc(c, "foo", DILL_I);
	dill_retii(c, 5);
	func = (int (*)())dill_end(c);
	if (verbose) dill_dump(c);
	result = func();

	if (result != 5) {
	    printf("Test 1 failed, got %d instead of 5\n", result);
	    exit(1);
	}
    }

    return 0;
}
