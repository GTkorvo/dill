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

static char *DILL_version = "DILL Version 1.0.134 rev. 11680  -- 2012-08-21 18:39:53 -0400 (Tue, 21 Aug 2012)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

