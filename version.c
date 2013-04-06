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

static char *DILL_version = "DILL Version 1.0.140 rev. 14074  -- 2013-04-05 19:14:11 -0400 (Fri, 05 Apr 2013)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

