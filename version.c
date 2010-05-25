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

static char *DILL_version = "DILL Version 1.0.98 rev. 8321  -- 2010-05-24 20:21:42 -0400 (Mon, 24 May 2010)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

