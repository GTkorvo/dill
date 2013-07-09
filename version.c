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

static char *DILL_version = "DILL Version 1.0.147 rev. 14953  -- 2013-07-08 13:52:47 -0400 (Mon, 08 Jul 2013)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

