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

static char *DILL_version = "DILL Version 1.0.117 rev. 10519  -- 2012-01-03 13:43:59 -0500 (Tue, 03 Jan 2012)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

