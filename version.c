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

static char *DILL_version = "DILL Version 1.0.119 rev. 10738  -- 2012-02-17 15:49:48 -0500 (Fri, 17 Feb 2012)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

