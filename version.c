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

static char *DILL_version = "DILL Version 1.0.130 rev. 11108  -- 2012-05-03 18:05:50 -0400 (Thu, 03 May 2012)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

