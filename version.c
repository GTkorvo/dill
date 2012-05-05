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

static char *DILL_version = "DILL Version 1.0.131 rev. 11118  -- 2012-05-04 09:57:23 -0400 (Fri, 04 May 2012)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

