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

static char *DILL_version = "DILL Version 1.0.120 rev. 10740  -- 2012-02-19 15:26:20 -0500 (Sun, 19 Feb 2012)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

