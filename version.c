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

static char *DILL_version = "DILL Version 1.0.113 rev. 9797  -- 2011-03-20 16:41:16 -0400 (Sun, 20 Mar 2011)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

