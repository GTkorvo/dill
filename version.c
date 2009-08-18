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

static char *DILL_version = "DILL Version 1.0.93 rev. 7489  -- 2009-08-17 19:46:43 -0400 (Mon, 17 Aug 2009)))))))\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

