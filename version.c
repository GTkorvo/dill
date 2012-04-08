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

static char *DILL_version = "DILL Version 1.0.125 rev. 10887  -- 2012-04-07 21:17:09 -0400 (Sat, 07 Apr 2012)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

