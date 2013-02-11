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

static char *DILL_version = "DILL Version 1.0.137 rev. 13316  -- 2013-02-10 19:48:48 -0500 (Sun, 10 Feb 2013)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

