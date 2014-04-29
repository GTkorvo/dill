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

static char *DILL_version = "DILL Version 2.1.7 rev. 17693  -- 2014-04-28 16:50:56 -0400 (Mon, 28 Apr 2014)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

