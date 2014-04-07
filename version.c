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

static char *DILL_version = "DILL Version 2.1.6 rev. 17438  -- 2014-04-06 07:36:17 -0400 (Sun, 06 Apr 2014)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

