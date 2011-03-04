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

static char *DILL_version = "DILL Version 1.0.110 rev. 9734  -- 2011-03-03 17:07:04 -0500 (Thu, 03 Mar 2011)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

