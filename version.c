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

static char *DILL_version = "DILL Version 2.1.15 rev. 21078  -- 2015-04-14 18:39:58 -0400 (Tue, 14 Apr 2015)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

