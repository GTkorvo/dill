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

static char *DILL_version = "DILL Version 1.0.101 rev. 8726  -- 2010-08-25 08:39:09 -0400 (Wed, 25 Aug 2010)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

