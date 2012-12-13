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

static char *DILL_version = "DILL Version 1.0.135 rev. 12596  -- 2012-11-09 12:21:37 -0500 (Fri, 09 Nov 2012)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

