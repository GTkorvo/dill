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

static char *DILL_version = "DILL Version 1.0.148 rev. 15045  -- 2013-07-25 14:24:32 -0400 (Thu, 25 Jul 2013)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

