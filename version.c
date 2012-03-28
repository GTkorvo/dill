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

static char *DILL_version = "DILL Version 1.0.124 rev. 10855  -- 2012-03-27 15:09:34 -0400 (Tue, 27 Mar 2012)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

