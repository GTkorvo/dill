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

static char *DILL_version = "DILL Version 1.0.142 rev. 14155  -- 2013-04-09 17:10:34 -0400 (Tue, 09 Apr 2013)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

