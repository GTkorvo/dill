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

static char *DILL_version = "DILL Version 2.1.18 rev. 21355  -- 2015-04-28 14:10:21 -0400 (Tue, 28 Apr 2015)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

