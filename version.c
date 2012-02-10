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

static char *DILL_version = "DILL Version 1.0.118 rev. 10736  -- 2012-02-09 21:48:57 -0500 (Thu, 09 Feb 2012)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

