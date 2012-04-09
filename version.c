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

static char *DILL_version = "DILL Version 1.0.126 rev. 10893  -- 2012-04-08 09:08:46 -0400 (Sun, 08 Apr 2012)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

