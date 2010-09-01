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

static char *DILL_version = "DILL Version 1.0.102 rev. 8741  -- 2010-08-31 21:44:49 -0400 (Tue, 31 Aug 2010)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

