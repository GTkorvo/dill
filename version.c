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

static char *DILL_version = "DILL Version 1.0.146 rev. 14781  -- 2013-06-10 14:10:53 -0400 (Mon, 10 Jun 2013)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

