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

static char *DILL_version = "DILL Version 1.0.99 rev. 8352  -- 2010-06-05 22:58:05 -0400 (Sat, 05 Jun 2010)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

