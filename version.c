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

static char *DILL_version = "DILL Version 1.0.127 rev. 10904  -- 2012-04-09 15:07:37 -0400 (Mon, 09 Apr 2012)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

