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

static char *DILL_version = "DILL Version 2.1.10 rev. 19025  -- 2014-10-24 14:21:41 -0400 (Fri, 24 Oct 2014)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

