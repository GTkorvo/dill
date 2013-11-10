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

static char *DILL_version = "DILL Version 1.0.149 rev. 16165  -- 2013-11-09 17:58:43 -0500 (Sat, 09 Nov 2013)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

