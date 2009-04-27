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

static char *DILL_version = "DILL Version 1.0.87 rev. 7050  -- 2009-02-13 08:43:27 -0500 (Fri, 13 Feb 2009)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

