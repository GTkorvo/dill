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

static char *DILL_version = "DILL Version 1.0.114 rev. 9801  -- 2011-03-21 17:02:57 -0400 (Mon, 21 Mar 2011)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

