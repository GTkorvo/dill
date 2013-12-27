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

static char *DILL_version = "DILL Version 2.1.2 rev. 16620  -- 2013-12-26 16:05:49 -0500 (Thu, 26 Dec 2013)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

