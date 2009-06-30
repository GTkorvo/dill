#if defined(FUNCPROTO) || defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)
#ifndef ARGS
#define ARGS(args) args
#endif
#else
#ifndef ARGS
#define ARGS(args) (/*args*/)
#endif
#endif

#include <stdio.h>
#include "config.h"

static char *DILL_version = "DILL Version 1.0.92 rev. 7454  -- 2009-06-29 14:34:49 -0400 (Mon, 29 Jun 2009))))))\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

