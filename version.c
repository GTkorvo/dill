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

static char *DILL_version = "DILL Version 2.1.1 rev. 16610  -- 2013-12-24 08:52:58 -0500 (Tue, 24 Dec 2013)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

