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

static char *DILL_version = "DILL Version 1.0.152 rev. 16605  -- 2013-12-22 10:22:50 -0500 (Sun, 22 Dec 2013)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

