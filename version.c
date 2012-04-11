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

static char *DILL_version = "DILL Version 1.0.128 rev. 10911  -- 2012-04-10 10:35:19 -0400 (Tue, 10 Apr 2012)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

