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

static char *DILL_version = "DILL Version 2.1.4 rev. 16880  -- 2014-02-04 22:22:28 -0500 (Tue, 04 Feb 2014)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

