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

static char *DILL_version = "DILL Version 1.0.112 rev. 9790  -- 2011-03-19 15:28:01 -0400 (Sat, 19 Mar 2011)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

