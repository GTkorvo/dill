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

static char *DILL_version = "DILL Version 1.0.133 rev. 11154  -- 2012-05-10 11:34:23 -0400 (Thu, 10 May 2012)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

