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

static char *DILL_version = "DILL Version 1.0.89 rev. 7396  -- 2009-05-12 20:40:44 -0400 (Tue, 12 May 2009)))\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

