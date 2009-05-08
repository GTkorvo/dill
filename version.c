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

static char *DILL_version = "DILL Version 1.0.88 rev. 7378  -- 2009-05-07 20:45:07 -0400 (Thu, 07 May 2009))\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

