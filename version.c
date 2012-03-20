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

static char *DILL_version = "DILL Version 1.0.122 rev. 10773  -- 2012-03-19 13:39:17 -0400 (Mon, 19 Mar 2012)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

