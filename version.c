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

static char *DILL_version = "DILL Version 1.0.150 rev. 16539  -- 2013-12-11 10:10:35 -0500 (Wed, 11 Dec 2013)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

