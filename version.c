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

static char *DILL_version = "DILL Version 1.0.138 rev. 13426  -- 2013-02-19 08:49:32 -0500 (Tue, 19 Feb 2013)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

