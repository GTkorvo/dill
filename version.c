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

static char *DILL_version = "DILL Version 2.1.8 rev. 17704  -- 2014-04-29 01:19:19 -0400 (Tue, 29 Apr 2014)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

