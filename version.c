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

static char *DILL_version = "DILL Version 1.0.139 rev. 13612  -- 2013-03-03 14:20:58 -0500 (Sun, 03 Mar 2013)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

