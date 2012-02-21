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

static char *DILL_version = "DILL Version 1.0.121 rev. 10748  -- 2012-02-20 21:01:20 -0500 (Mon, 20 Feb 2012)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

