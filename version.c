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

static char *DILL_version = "DILL Version 2.1.19 rev. 26377  -- 2017-01-03 13:41:18 -0500 (Tue, 03 Jan 2017)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

