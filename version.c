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

static char *DILL_version = "DILL Version 2.1.16 rev. 21336  -- 2015-04-24 21:03:42 -0400 (Fri, 24 Apr 2015)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

