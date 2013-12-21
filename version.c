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

static char *DILL_version = "DILL Version 1.0.151 rev. 16592  -- 2013-12-20 10:28:35 -0500 (Fri, 20 Dec 2013)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

