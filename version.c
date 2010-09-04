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

static char *DILL_version = "DILL Version 1.0.104 rev. 8758  -- 2010-09-03 06:17:27 -0400 (Fri, 03 Sep 2010)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

