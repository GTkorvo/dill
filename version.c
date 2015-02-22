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

static char *DILL_version = "DILL Version 2.1.13 rev. 20432  -- 2015-02-21 10:53:12 -0500 (Sat, 21 Feb 2015)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

