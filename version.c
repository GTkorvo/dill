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

static char *DILL_version = "DILL Version 1.0.136 rev. 13162  -- 2013-01-29 13:59:29 -0500 (Tue, 29 Jan 2013)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

