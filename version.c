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

static char *DILL_version = "DILL Version 2.1.5 rev. 16889  -- 2014-02-05 15:40:29 -0500 (Wed, 05 Feb 2014)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

