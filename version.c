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

static char *DILL_version = "DILL Version 2.1.20 rev. 26894  -- 2017-05-29 08:59:54 -0400 (Mon, 29 May 2017)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

