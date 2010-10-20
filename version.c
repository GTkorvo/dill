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

static char *DILL_version = "DILL Version 1.0.105 rev. 8935  -- 2010-10-19 16:07:55 -0400 (Tue, 19 Oct 2010)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

