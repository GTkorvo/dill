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

static char *DILL_version = "DILL Version 2.1.17 rev. 21344  -- 2015-04-27 09:56:59 -0400 (Mon, 27 Apr 2015)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

