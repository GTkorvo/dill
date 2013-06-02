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

static char *DILL_version = "DILL Version 1.0.144 rev. 14757  -- 2013-06-01 10:03:37 -0400 (Sat, 01 Jun 2013)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

